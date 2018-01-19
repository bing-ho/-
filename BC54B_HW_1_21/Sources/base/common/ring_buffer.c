/// \file ring_buffer.c
/// \brief 循环Buffer的实现.
/// \author Xi Qingping, qingping.xi@ligoo.cn
/// \version
/// \date 2015-09-01
//

#include "ring_buffer.h"

#if RINGBUFFER_CHECK != 0
#define RINGBUFFER_IS_VALID(rb) ((0 != (rb)) && ((rb)->magic == RINGBUFFER_CHECK_MAGIC))
#else
#define RINGBUFFER_IS_VALID(rb) (0 != (rb))
#endif

void RingBuffer_Clear(ring_buffer_t *__FAR rb) {
    if (!RINGBUFFER_IS_VALID(rb)) {
        return;
    }

    rb->len = 0;
    rb->in = 0;
    rb->out = 0;
}

RINGBUFFER_SIZE_TYPE RingBuffer_PutData(ring_buffer_t *__FAR rb, unsigned char uch, RINGBUFFER_SIZE_TYPE len) {
    RINGBUFFER_SIZE_TYPE i;

    if (!RINGBUFFER_IS_VALID(rb)) {
        return 0;
    }

    for (i = 0; (i < len) && (rb->len < rb->cap); ++i) {
        rb->buf[rb->in++] = uch;
        if (rb->in >= rb->cap) {
            rb->in = 0;
        }
        ++rb->len;
    }
    return i;
}

RINGBUFFER_SIZE_TYPE RingBuffer_TryRead(ring_buffer_t *__FAR rb, unsigned char *__FAR buf, RINGBUFFER_SIZE_TYPE len) {
    RINGBUFFER_SIZE_TYPE i;

    if (!RINGBUFFER_IS_VALID(rb)) {
        return 0;
    }
    
    if (!buf) {
        return 0;
    }

    for (i = 0; (i < len) && (rb->len > 0); ++i) {
        if (0 != buf) {
            *buf++ = rb->buf[rb->out];
        }
        ++rb->out;
        if (rb->out >= rb->cap) {
            rb->out = 0;
        }
        --rb->len;
    }

    return i;
}

RINGBUFFER_SIZE_TYPE RingBuffer_TryWrite(ring_buffer_t *__FAR rb, const unsigned char *__FAR dat, RINGBUFFER_SIZE_TYPE len) {
    RINGBUFFER_SIZE_TYPE i;

    if (!RINGBUFFER_IS_VALID(rb)) {
        return 0;
    }

    if (!dat) {
        return 0;
    }

    for (i = 0; (i < len) && (rb->len < rb->cap); ++i) {
        rb->buf[rb->in++] = *dat++;
        if (rb->in >= rb->cap) {
            rb->in = 0;
        }
        ++rb->len;
    }
    return i;
}

RINGBUFFER_SIZE_TYPE RingBuffer_Read(ring_buffer_t *__FAR rb, unsigned char *__FAR buf, RINGBUFFER_SIZE_TYPE len) {
    if (!RINGBUFFER_IS_VALID(rb)) {
        return 0;
    }

    if ((rb->cap - rb->len) < len) {
        return 0;
    }

    return RingBuffer_TryRead(rb, buf, len);
}

RINGBUFFER_SIZE_TYPE RingBuffer_Write(ring_buffer_t *__FAR rb, const unsigned char *__FAR dat, RINGBUFFER_SIZE_TYPE len) {
    if (!RINGBUFFER_IS_VALID(rb)) {
        return 0;
    }

    if ((rb->cap - rb->len) < len) {
        return 0;
    }

    return RingBuffer_TryWrite(rb, dat, len);
}

RINGBUFFER_SIZE_TYPE RingBuffer_LeftSpace(const ring_buffer_t *__FAR rb) {
    if (!RINGBUFFER_IS_VALID(rb)) {
        return 0;
    }

    return (rb->cap - rb->len);
}

static unsigned char pop_char(ring_buffer_t *__FAR rb, char *__FAR pc) {
    if (rb->len == 0) {
        return 0;
    }
    *pc = rb->buf[rb->out++];
    if (rb->out >= rb->cap) {
        rb->out = 0;
    }
    --rb->len;
    return 1;
}
RINGBUFFER_SIZE_TYPE RingBuffer_PopString(ring_buffer_t *__FAR rb, char *__FAR buf, RINGBUFFER_SIZE_TYPE bufSize) {
    char chr;
    RINGBUFFER_SIZE_TYPE ret = 0;
    
    if (!RINGBUFFER_IS_VALID(rb)) {
        return 0;
    }

    do {
        if (0 == pop_char(rb, &chr)) {
            break;
        }
        if (buf && ret < bufSize) {
            buf[ret++] = chr;
        }
    } while (chr != 0);

    return ret;
}

char RingBuffer_PopStringIsStartWith(ring_buffer_t *__FAR rb, const char *__FAR prefix, char *__FAR buf, RINGBUFFER_SIZE_TYPE *__FAR size) {
    if (!RINGBUFFER_IS_VALID(rb)) {
        return 0;
    }

    if (!size) return 0;

    if (!prefix) {
        *size = RingBuffer_PopString(rb, buf, *size);
        return (*size  > 0);
    } else {
        char chr = 1;
        const char *__FAR pe;
        RINGBUFFER_SIZE_TYPE saved = 0;

        for (pe = prefix; 0 != *pe; ++pe) {
            if (0 == pop_char(rb, &chr)) {
                *size = saved;
                return 0;
            }
            if (buf && saved < *size) {
                buf[saved++] = chr;
            }
            if (*pe != chr) {
                if (0 != chr) {
                    saved += RingBuffer_PopString(rb, &buf[saved], *size - saved);
                }
                break;
            }
        }

        if (*pe == 0) {
            if (chr != 0) {
                saved += RingBuffer_PopString(rb, &buf[saved], *size - saved);
            } else if (0 != buf && saved < *size) {
                buf[saved] = 0;
            }

            *size = saved;
            return 1;
        }

        *size = saved;
        return 0;
    }
}

RINGBUFFER_SIZE_TYPE RingBuffer_DropDataFromTail(ring_buffer_t *__FAR rb, RINGBUFFER_SIZE_TYPE size) {
    if (!RINGBUFFER_IS_VALID(rb)) {
        return 0;
    }

    if (size == 0) {
        return 0;
    }
    if (size < rb->len) {
        size = rb->len;
    }
    if (rb->out < size) {
        rb->out = rb->out + rb->cap  - size;
    } else {
        rb->out -= size;
    }

    rb->len -= size;
    return size;
}

RINGBUFFER_SIZE_TYPE RingBuffer_DropDataFromHead(ring_buffer_t *__FAR rb, RINGBUFFER_SIZE_TYPE size) {
    if (!RINGBUFFER_IS_VALID(rb)) {
        return 0;
    }

    if (size < rb->len) {
        size = rb->len;
    }

    rb->in += size;
    if (rb->in >= rb->cap) {
        rb->in -= rb->cap;
    }

    rb->len -= size;
    return size;
}

