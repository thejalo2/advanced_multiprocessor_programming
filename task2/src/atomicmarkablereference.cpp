#include "atomicmarkablereference.hpp"

#define TID(id) *((uint64_t *)&id)

AtomicMarkableReference::
AtomicMarkableReference()
{
    static_assert(sizeof(uint64_t) == sizeof(std::thread::id),
            "This class heavily relies on std::thread::id being "
            "of equal size as uint64_t");
    the_value = 0;
}

AtomicMarkableReference::
AtomicMarkableReference(std::thread::id reference,
                        bool mark)
{
    static_assert(sizeof(uint64_t) == sizeof(std::thread::id),
            "This class heavily relies on std::thread::id being "
            "of equal size as uint64_t");
    the_value = (uint64_t)mark | TID(reference) << 1;
}

bool
AtomicMarkableReference::
compareAndSet(const std::thread::id expectedReference,
              const std::thread::id newReference,
              const bool expectedMark,
              const bool newMark)
{
    return compareAndSet(TID(expectedReference),
                         TID(newReference),
                         expectedMark,
                         newMark);
}

bool
AtomicMarkableReference::
attemptMark(const std::thread::id newReference,
            const bool newMark)
{
    return compareAndSet(0, TID(newReference), false, newMark);
}

void
AtomicMarkableReference::
reset()
{
    the_value = 0;
}

std::thread::id
AtomicMarkableReference::
get(bool *mark)
{
    *mark = the_value & 1;
    uint64_t val = (the_value >> 1);
    return  ((std::thread::id)val);
}

bool
AtomicMarkableReference::
compareAndSet(const uint64_t expectedReference,
              const uint64_t newReference,
              const bool expectedMark,
              const bool newMark)
{
    /* TODO: casting std::thread::id to uint64_t might be dangerous!!
     * std::thread::id does not give any guarantees except for it to
     * be unique for each thread. */
    uint64_t expected = (uint64_t)expectedMark | expectedReference << 1;
    const uint64_t update = (uint64_t)newMark | newReference << 1;

    return the_value.compare_exchange_strong(expected, update);
}
