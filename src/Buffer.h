#ifndef HALIDE_BUFFER_H
#define HALIDE_BUFFER_H


#include "buffer_t.h"
#include "JITCompiledModule.h"
#include "IntrusivePtr.h"
#include "Type.h"
#include "Argument.h"
#include "Util.h"

#include <stdint.h>

/** \file
 * Defines Buffer - A c++ wrapper around a buffer_t.
 */

namespace Halide {
namespace Internal {

struct BufferContents {
    /** The buffer_t object we're wrapping. */
    buffer_t buf;

    /** The type of the allocation. buffer_t's don't currently track this so we do it here. */
    Type type;

    /** If we made the allocation ourselves via a Buffer constructor,
     * and hence should delete it when this buffer dies, then this
     * pointer is set to the memory we need to free. Otherwise it's
     * NULL. */
    uint8_t *allocation;

    /** How many Buffer objects point to this BufferContents */
    mutable RefCount ref_count;

    /** What is the name of the buffer? Useful for debugging symbols. */
    std::string name;

    /** If this buffer was generated by a jit-compiled module, we need
     * to hang onto the module, as it may contain internal state that
     * tells us how to use this buffer. In particular, if the buffer
     * was generated on the gpu and hasn't been copied back yet, only
     * the module knows how to do that, so we'd better keep the module
     * alive. */
    JITCompiledModule source_module;

    BufferContents(Type t, int x_size, int y_size, int z_size, int w_size,
                   uint8_t* data, const std::string &n) :
        type(t), allocation(NULL), name(n.empty() ? unique_name('b') : n) {
        assert(t.width == 1 && "Can't create of a buffer of a vector type");
        buf.elem_size = t.bytes();
        size_t size = 1;
        if (x_size) size *= x_size;
        if (y_size) size *= y_size;
        if (z_size) size *= z_size;
        if (w_size) size *= w_size;
        if (!data) {
            size = buf.elem_size*size + 32;
            allocation = (uint8_t *)calloc(1, size);
            buf.host = allocation;
            while ((size_t)(buf.host) & 0x1f) buf.host++;
        } else {
            buf.host = data;
        }
        buf.dev = 0;
        buf.host_dirty = false;
        buf.dev_dirty = false;
        buf.extent[0] = x_size;
        buf.extent[1] = y_size;
        buf.extent[2] = z_size;
        buf.extent[3] = w_size;
        buf.stride[0] = 1;
        buf.stride[1] = x_size;
        buf.stride[2] = x_size*y_size;
        buf.stride[3] = x_size*y_size*z_size;
        buf.min[0] = 0;
        buf.min[1] = 0;
        buf.min[2] = 0;
        buf.min[3] = 0;
    }

    BufferContents(Type t, const buffer_t *b, const std::string &n) :
        type(t), name(n.empty() ? unique_name('b') : n) {
        buf = *b;
        assert(t.width == 1 && "Can't create of a buffer of a vector type");
    }
};
}

/** The internal representation of an image, or other dense array
 * data. The Image type provides a typed view onto a buffer for the
 * purposes of direct manipulation. A buffer may be stored in main
 * memory, or some other memory space (e.g. a gpu). If you want to use
 * this as an Image, see the Image class. Casting a Buffer to an Image
 * will do any appropriate copy-back. This class is a fairly thin
 * wrapper on a buffer_t, which is the C-style type Halide uses for
 * passing buffers around.
 */
class Buffer {
private:
    Internal::IntrusivePtr<Internal::BufferContents> contents;
public:
    Buffer() : contents(NULL) {}

    Buffer(Type t, int x_size = 0, int y_size = 0, int z_size = 0, int w_size = 0,
           uint8_t* data = NULL, const std::string &name = "") :
        contents(new Internal::BufferContents(t, x_size, y_size, z_size, w_size, data, name)) {
    }

    Buffer(Type t, const buffer_t *buf, const std::string &name = "") :
        contents(new Internal::BufferContents(t, buf, name)) {
    }

    /** Get a pointer to the host-side memory. */
    void *host_ptr() const {
        assert(defined());
        return (void *)contents.ptr->buf.host;
    }

    /** Get a pointer to the raw buffer_t struct that this class wraps. */
    buffer_t *raw_buffer() const {
        assert(defined());
        return &(contents.ptr->buf);
    }

    /** Get the device-side pointer/handle for this buffer. Will be
     * zero if no device was involved in the creation of this
     * buffer. */
    uint64_t device_handle() const {
        assert(defined());
        return contents.ptr->buf.dev;
    }

    /** Has this buffer been modified on the cpu since last copied to a
     * device. Not meaningful unless there's a device involved. */
    bool host_dirty() const {
        assert(defined());
        return contents.ptr->buf.host_dirty;
    }

    /** Let Halide know that the host-side memory backing this buffer
     * has been externally modified. You shouldn't normally need to
     * call this, because it is done for you when you cast a Buffer to
     * an Image in order to modify it. */
    void set_host_dirty(bool dirty = true) {
        assert(defined());
        contents.ptr->buf.host_dirty = dirty;
    }

    /** Has this buffer been modified on device since last copied to
     * the cpu. Not meaninful unless there's a device involved. */
    bool device_dirty() const {
        assert(defined());
        return contents.ptr->buf.dev_dirty;
    }

    /** Let Halide know that the device-side memory backing this
     * buffer has been externally modified, and so the cpu-side memory
     * is invalid. A copy-back will occur the next time you cast this
     * Buffer to an Image, or the next time this buffer is accessed on
     * the host in a halide pipeline. */
    void set_device_dirty(bool dirty = true) {
        assert(defined());
        contents.ptr->buf.host_dirty = dirty;
    }

    /** Get the dimensionality of this buffer. Uses the convention
     * that the extent field of a buffer_t should contain zero when
     * the dimensions end. */
    int dimensions() const {
        for (int i = 0; i < 4; i++) {
            if (extent(i) == 0) return i;
        }
        return 4;
    }

    /** Get the extent of this buffer in the given dimension. */
    int extent(int dim) const {
        assert(defined());
        assert(dim >= 0 && dim < 4 && "We only support 4-dimensional buffers for now");
        return contents.ptr->buf.extent[dim];
    }

    /** Get the number of bytes between adjacent elements of this buffer along the given dimension. */
    int stride(int dim) const {
        assert(defined());
        assert(dim >= 0 && dim < 4 && "We only support 4-dimensional buffers for now");
        return contents.ptr->buf.stride[dim];
    }

    /** Get the coordinate in the function that this buffer represents
     * that corresponds to the base address of the buffer. */
    int min(int dim) const {
        assert(defined());
        assert(dim >= 0 && dim < 4 && "We only support 4-dimensional buffers for now");
        return contents.ptr->buf.min[dim];
    }

    /** Set the coordinate in the function that this buffer represents
     * that corresponds to the base address of the buffer. */
    void set_min(int m0, int m1 = 0, int m2 = 0, int m3 = 0) {
        assert(defined());
        contents.ptr->buf.min[0] = m0;
        contents.ptr->buf.min[1] = m1;
        contents.ptr->buf.min[2] = m2;
        contents.ptr->buf.min[3] = m3;
    }

    /** Get the Halide type of the contents of this buffer. */
    Type type() const {
        assert(defined());
        return contents.ptr->type;
    }

    /** Compare two buffers for identity (not equality of data). */
    bool same_as(const Buffer &other) const {
        return contents.same_as(other.contents);
    }

    /** Check if this buffer handle actually points to data. */
    bool defined() const {
        return contents.defined();
    }

    /** Get the runtime name of this buffer used for debugging. */
    const std::string &name() const {
        return contents.ptr->name;
    }

    /** Convert this buffer to an argument to a halide pipeline. */
    operator Argument() const {
        return Argument(name(), true, type());
    }

    /** Declare that this buffer was created by the given jit-compiled
     * module. Used internally for reference counting the module. */
    void set_source_module(const Internal::JITCompiledModule &module) {
        assert(defined());
        contents.ptr->source_module = module;
    }

    /** If this buffer was the output of a jit-compiled realization,
     * retrieve the module it came from. Otherwise returns a module
     * struct full of null pointers. */
    const Internal::JITCompiledModule &source_module() {
        assert(defined());
        return contents.ptr->source_module;
    }

    /** If this buffer was created *on-device* by a jit-compiled
     * realization, then copy it back to the cpu-side memory. This is
     * usually achieved by casting the Buffer to an Image. */
    void copy_to_host() {
        void (*copy_to_host)(buffer_t *) =
            contents.ptr->source_module.copy_to_host;
        if (copy_to_host) {
            copy_to_host(raw_buffer());
        }
    }

    /** If this buffer was created by a jit-compiled realization on a
     * device-aware target (e.g. PTX), then copy the cpu-side data to
     * the device-side allocation. TODO: I believe this currently
     * aborts messily if no device-side allocation exists. You might
     * think you want to do this because you've modified the data
     * manually on the host before calling another Halide pipeline,
     * but what you actually want to do in that situation is set the
     * host_dirty bit so that Halide can manage the copy lazily for
     * you. Casting the Buffer to an Image sets the dirty bit for
     * you. */
    void copy_to_dev() {
        void (*copy_to_dev)(buffer_t *) =
            contents.ptr->source_module.copy_to_dev;
        if (copy_to_dev) {
            copy_to_dev(raw_buffer());
        }
    }

    /** If this buffer was created by a jit-compiled realization on a
     * device-aware target (e.g. PTX), then free the device-side
     * allocation, if there is one. Done automatically when the last
     * reference to this buffer dies. */
    void free_dev_buffer() {
        void (*free_dev_buffer)(buffer_t *) =
            contents.ptr->source_module.free_dev_buffer;
        if (free_dev_buffer) {
            free_dev_buffer(raw_buffer());
        }
    }

};

namespace Internal {
template<>
inline RefCount &ref_count<BufferContents>(const BufferContents *p) {
    return p->ref_count;
}

template<>
inline void destroy<BufferContents>(const BufferContents *p) {
    // Free any device-side allocation
    if (p->source_module.free_dev_buffer) {
        p->source_module.free_dev_buffer(const_cast<buffer_t *>(&p->buf));
    }
    if (p->allocation) {
        free(p->allocation);
    }
    delete p;
}

}
}

#endif
