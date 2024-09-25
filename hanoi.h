#pragma once

#include <stdbool.h>

#define HANOI_PIECES 5
#define HANOI_ROWS (HANOI_PIECES+1)
#define HANOI_TOWER_WIDTH 10
#define HANOI_TOWER_GAP 1

void hanoi_putc(char ch, unsigned n) {
    if (n > 1024)
        return;
    while (n--)
        putchar(ch);
}

uint32_t rand() {
    static uint32_t seed = 123456789;
    if (seed & 1)
        seed = (seed>>1) ^ 0xA000001U;
    else
        seed = seed>>1;
    return seed;
}

typedef struct hanoi_context_s {
    uint8_t towers[3][HANOI_ROWS];
    uint8_t holding, holding_over;
} hanoi_context_t;

unsigned hanoi_widest_piece(hanoi_context_t *ctx, int pin) {
    return ctx->towers[pin][0];
}

unsigned hanoi_tower_width(hanoi_context_t *ctx, int pin) {
    unsigned W = hanoi_widest_piece(ctx,pin);

    if (ctx->holding_over == pin && W < ctx->holding)
        W = ctx->holding;

    if (W < HANOI_PIECES-1)
        W = HANOI_PIECES-1;

    return 2*W+1;
}

void hanoi_print_piece(unsigned W, unsigned piece) {
    hanoi_putc(' ', W/2 - piece);
    hanoi_putc('X',   1 + piece * 2);
    hanoi_putc(' ', W/2 - piece);
}

void hanoi_print_pin(unsigned W) {
    hanoi_putc(' ', W/2);
    hanoi_putc('|',   1);
    hanoi_putc(' ', W/2);
}

void hanoi_print_hand(unsigned W, unsigned piece) {
    if (piece) {
        hanoi_print_piece(W, piece);
        return;
    }

    hanoi_putc(' ', W/2-3);
    hanoi_putc('(', 1);
    hanoi_putc(' ', 5);
    hanoi_putc(')', 1);
    hanoi_putc(' ', W/2-3);
}

void hanoi_print_row_pin(hanoi_context_t *ctx, unsigned row, unsigned pin) {
    unsigned piece = ctx->towers[pin][row];
    unsigned W = hanoi_tower_width(ctx, pin);

    if (piece)
        hanoi_print_piece(W, piece);
    else
        hanoi_print_pin(W);
}

void hanoi_print_row(hanoi_context_t *ctx, unsigned row) {
    hanoi_print_row_pin(ctx, row, 0);
    hanoi_putc(' ', HANOI_TOWER_GAP);
    hanoi_print_row_pin(ctx, row, 1);
    hanoi_putc(' ', HANOI_TOWER_GAP);
    hanoi_print_row_pin(ctx, row, 2);
    hanoi_putc('\n', 1);
}

void hanoi_print_hand_row(hanoi_context_t *ctx) {
    unsigned W;

    W = hanoi_tower_width(ctx, 0);
    if (ctx->holding_over == 0)
        hanoi_print_hand(W, ctx->holding);
    else
        hanoi_putc(' ', W);

    hanoi_putc(' ', HANOI_TOWER_GAP);

    W = hanoi_tower_width(ctx, 1);
    if (ctx->holding_over == 1)
        hanoi_print_hand(W, ctx->holding);
    else
        hanoi_putc(' ', W);

    hanoi_putc(' ', HANOI_TOWER_GAP);

    W = hanoi_tower_width(ctx, 2);
    if (ctx->holding_over == 2)
        hanoi_print_hand(W, ctx->holding);
    else
        hanoi_putc(' ', W);

    hanoi_putc('\n', 1);
}

void hanoi_print_pins(hanoi_context_t *ctx) {
    for (int row = HANOI_ROWS; row--;)
        hanoi_print_row(ctx, row);

    unsigned W = 2*HANOI_TOWER_GAP
        + hanoi_tower_width(ctx, 0)
        + hanoi_tower_width(ctx, 1)
        + hanoi_tower_width(ctx, 2);

    hanoi_putc('=', W);
    hanoi_putc('\n', 1);
}

void hanoi_print(hanoi_context_t *ctx) {
    hanoi_putc('\f',1);
    hanoi_print_hand_row(ctx);
    hanoi_putc('\n',1);
    hanoi_print_pins(ctx);
}

bool hanoi_place_at(hanoi_context_t *ctx, unsigned pin, unsigned piece) {
    for (unsigned pos=0; pos<HANOI_ROWS; pos++) {
        uint8_t *p = &ctx->towers[pin][pos];
        if (*p) {
            // If tower contains a smaller piece. Can not place;
            if (*p < piece)
                return false;
        } else {
            // Place the piece
            *p = piece;
            return true;
        }
    }
    // Tower has not enough space. Should never happen.
    return false;
}

unsigned hanoi_take_from(hanoi_context_t *ctx, unsigned pin) {
    for (unsigned pos=HANOI_ROWS; pos--;) {
        uint8_t *p = &ctx->towers[pin][pos];
        if (!*p)
            continue;

        // Take piece
        unsigned piece = *p;
        *p = 0;
        return piece;
    }
    // No pieces to take
    return 0;
}

bool hanoi_place(hanoi_context_t *ctx) {
    if (!ctx->holding)
        return false;

    if (!hanoi_place_at(ctx, ctx->holding_over, ctx->holding))
        return false;

    ctx->holding = 0;
    return true;
}

bool hanoi_take(hanoi_context_t *ctx) {
    if (ctx->holding)
        return false;

    ctx->holding = hanoi_take_from(ctx, ctx->holding_over);
    return !!ctx->holding;
}

void hanoi_clean(hanoi_context_t *ctx) {
    memset(ctx->towers, 0, sizeof(ctx->towers));
    ctx->holding = 0;
    ctx->holding_over = 0;
}

void hanoi_randomize(hanoi_context_t *ctx) {
    hanoi_clean(ctx);

    for (int i=0; i<HANOI_PIECES; ++i) {
        int pin = rand()%3;
        int piece = HANOI_PIECES-i;

        hanoi_place_at(ctx, pin, piece);
    }

    ctx->holding_over = rand() % 3;
    if (rand() % 2)
        hanoi_take(ctx);
}

void hanoi_main(uint8_t *VRAM) {
    hanoi_context_t ctx;

    // Test the worst case for dynamic width
    hanoi_clean(&ctx);
    for (int i=0; i<HANOI_PIECES; ++i) {
        int pin = i%3;
        int piece = HANOI_PIECES-i;

        hanoi_place_at(&ctx, pin, piece);
    }
    hanoi_print(&ctx);
    Delay_Ms(5000);

    // Test random states
    for (int i=0; i<60; i++) {
        hanoi_randomize(&ctx);
        hanoi_print(&ctx);
        Delay_Ms(1000);
    }
}
