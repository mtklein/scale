#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint16_t u16(uint64_t x) {
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
        tol = (n == 0 || n == d || d == 0 || d == 0xffff) ? 0 : 1;
    if (err > +tol || err < -tol) {
        dprintf(2, "n %u, d %u, want %d, got %d\n", n,d,want,got);
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
    // Here we try all k in range 16 to 48 (16 or 32 are probably fastest) and
    // let m = (0xffff << k)/d + 1, where the extra +1 helps some of the n == d
    // cases round correctly instead of producing 0xfffe.
    for (int k = 16; k <= 48; k++) {
        for (uint32_t d = 1; d <= 0xffff; d++) {
            uint64_t const m = ((uint64_t)0xffff << k) / d + 1;

            for (uint32_t n = 0; n <= d; n++) {
                uint16_t want = u16(n*0xffff / d),
                          got = u16(n*m   >>   k);
                check(n,d, want,got);
            }
        }
        dprintf(1, "k=%d ok\n", k);
    }

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
