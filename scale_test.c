#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint16_t u16(uint32_t x) {
    assert(x < 0x10000);
    return (uint16_t)x;
}

static uint16_t u16f(float x) {
    assert(0 <= x && x < (float)0x10000);
    return (uint16_t)x;
}

static void check(uint32_t n, uint32_t d, uint16_t want, uint16_t got) {
    // We allow 1 bit of error unless n or d is an edge case, which have to be perfect.
    int err = (int)want - (int)got,
        tol = (n == 0 || n == d || d == 1 || d == 0xffff) ? 0 : 1;
    if (err > +tol || err < -tol) {
        dprintf(2, "n %x, d %x, want %x, got %x\n", n,d,want,got);
        abort();
    }
}

int main(void) {
    // Let n = (val - min),
    // and d = (max - min).
    //
    // d can range 0 (max == min) to 0xffff (max == 0xffff, min == 0)
    // n can range 0 (val == min) to d (val == max)
    //
    // ... but also, d > 0 or this doesn't make sense.
    //
    // Then n*0xffff/d ≈ (n * (0xffff << k)/d) >> k for arbitrary k >= 16.
    //
    // If we pick k=16, we can just barely fit this in uint32_t.
    // And we round (0xffff << 16)/d up to 0xffffffff/d for better edge-case rounding.
    for (uint32_t d = 1; d <= 0xffff; d++) {
        uint32_t const m = 0xffffffff/d;

        for (uint32_t n = 0; n <= d; n++) {
            uint16_t want = u16(n*0xffff / d),
                      got = u16(n*m   >>  16);
            check(n,d, want,got);
        }
    }
    dprintf(1, "fixed point ok\n");

    // Using floats for this is basically as straightforward as you'd think.
    for (uint32_t d = 1; d <= 0xffff; d++) {
        float const invd = (float)0xffff / (float)d;

        for (uint32_t n = 0; n <= d; n++) {
            uint16_t want = u16 (n*0xffff / d),
                      got = u16f((float)n * invd + 0.5f);
            check(n,d, want,got);
        }
    }
    dprintf(1, "float ok\n");

    return 0;
}
