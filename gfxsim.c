#include <stdbool.h>
#include <SDL.h>

#define WIDTH 1024
#define HEIGHT 768

// 1 if bit b in v is set, 0 otherwise
#define BIT(v, b) (((v) & (1 << b)) ? 1 : 0)
#define BYTE(v) ((v) & 0xff)
#define NIBB(v) ((v) & 0x0f)
#define INVBYTE(v) (BYTE(v) ^ 0xff)
#define INVNIBB(v) (NIBB(v) ^ 0x0f)
	     
static unsigned int t = 0;

static void render_frame(SDL_Renderer *renderer) {
  unsigned int i, x, y;
  unsigned int px, py, c1, c2, carry;
  unsigned int plane1, plane2, plane3, plane1l;
  uint8_t yl, y8to1;
  uint8_t r, g, b;

  for(y = 0; y < HEIGHT; ++y) {
    for(x = 0; x < WIDTH/4; ++x) {
      yl = BYTE(y >> 2);	// Lower resolution y (y9 to y2) for 192 lines
      y8to1 = BYTE(y >> 1);	// y8 to y1, same as 2*yl but at higher res

      c1 = BIT(BYTE(BYTE(x) + BYTE(y8to1)) + BYTE(4*t), 7);	// 4 adders
      c2  = BIT(INVBYTE(2*x) + BYTE(yl), 7);			// 2 adders
      plane1 = !(c1 || c2);
      plane1l = c1 && c2;

      px = (NIBB(x) + INVNIBB(yl/4)) & 0x1f;	// 1 adder
      carry = BIT(px, 4);
      px = (NIBB(px) + NIBB(t)) & 0x1f;		// 1 adder
      // Compensate for Bit 4 missing in 4 bit adder (Bits 0-3)
      px ^= 16 * BIT(x, 4);
      px ^= 16 * BIT(yl/4, 4);
      px ^= 16 * BIT(t, 4);
      px ^= 16 * carry;
      
      py = ((NIBB(x/4) + NIBB(yl)) & 0x1f);	// 1 adder
      // Compensate for Bit 4 missing in 4 bit adder (Bits 0-3)
      py ^= 16 * BIT(x/4, 4);
      py ^= 16 * BIT(y/4, 4);
	
      plane2 = (BIT(px, 4) ^ BIT(py, 4));	// use carry
      plane3 = (BIT(x, 3) ^ BIT(yl + t, 3));	// 1 adder
      
      r = g = b = 0;
      if(plane2) {
	// Red and green channel use 5 bits from px/py padded with 0:
	// PPPPP000
	r = BYTE(px << 3);
	g = BYTE(py << 3);
      } else if(plane3) {
	b = 128;
	g = 64;
      }
      if(plane1l) {
	r |= 128;
	g /= 2;
	b = 64;
      }
      if(plane1) {
	r &= 127;	// clear MSB
	g /= 2;
	b = 128;
      }

      // Only connect the 5 MSBs per color channel to DAC
      r &= 0xf8;
      g &= 0xf8;
      b &= 0xf8;

      SDL_SetRenderDrawColor(renderer, r, g, b, 255);
      // Draw the same color for 4 pixels to account for lower pixel clock
      for(i = 0; i < 4; ++i) {
	SDL_RenderDrawPoint(renderer, (x << 2) + i, y);
      }
    }
  }
}

int main() {
  bool done = false;
  SDL_Event event;
  SDL_Window *window;
  SDL_Renderer *renderer;
  
  SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer);

  while(!done) {
    while(SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
	done = true;
      }
    }
    render_frame(renderer);
    SDL_RenderPresent(renderer);
    t = (t+1) % 256;
  }

  return EXIT_SUCCESS;
}
