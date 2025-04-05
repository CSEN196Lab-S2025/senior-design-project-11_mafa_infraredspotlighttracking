#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUM_anchors 4  // Number of anchors
#define MAX_TAGS 10  // Max number of tags to track (you can increase this as needed)

// Structure to store a point in 3D space
typedef struct {
    float x, y, z;
} Point;

// Structure to store tag information
typedef struct {
    char id[5]; // Tag ID (e.g., "048F")
    Point position;
} Tag;

// Function to parse tag positions from a file and update tags dynamically
int parse_coordinates(const char* filename, Tag tags[], int *num_tags) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        char tag_id[5];
        float x, y, z;
        int match = sscanf(line, "POS,0,%4s,%f,%f,%f", tag_id, &x, &y, &z);
        if (match == 4) {
            int found = 0;
            // Check if the tag already exists, if so, update its position
            for (int i = 0; i < *num_tags; i++) {
                if (strcmp(tags[i].id, tag_id) == 0) {
                    tags[i].position.x = x;
                    tags[i].position.y = y;
                    tags[i].position.z = z;
                    found = 1;
                    break;
                }
            }

            // If the tag doesn't exist, add it to the list
            if (!found && *num_tags < MAX_TAGS) {
                strcpy(tags[*num_tags].id, tag_id);
                tags[*num_tags].position.x = x;
                tags[*num_tags].position.y = y;
                tags[*num_tags].position.z = z;
                (*num_tags)++;
            }
        }
    }

    fclose(file);
    return 1;
}

// Function to initialize SDL and create a window
int init_sdl(SDL_Window **window, SDL_Renderer **renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    *window = SDL_CreateWindow("Tag Tracking",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               640, 480, SDL_WINDOW_SHOWN);
    if (*window == NULL) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL) {
        printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 1;
    }

    return 0;
}

// Function to draw the anchors as fixed points (scaled to the window size)
void draw_anchors(SDL_Renderer *renderer, Point anchors[], float scale_factor) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Green color for anchors
    for (int i = 0; i < NUM_anchors; i++) {
        // Apply the same scaling factor to the anchor positions
        int screen_x = (int)(anchors[i].x * scale_factor + 320);
        int screen_y = (int)(240 - anchors[i].y * scale_factor);  // Invert y to fit screen coordinates

        // Draw a bigger point for the anchor (adjust size as needed)
        SDL_RenderDrawPoint(renderer, screen_x, screen_y);
        SDL_Rect rect = {screen_x - 5, screen_y - 5, 10, 10};  // 10x10 rectangle for a bigger anchor dot
        SDL_RenderFillRect(renderer, &rect);
    }
}

// Function to update and draw tags (scaled to the window size)
void update_tags(SDL_Renderer *renderer, Tag tags[], int num_tags, float scale_factor) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red color for tags

    for (int i = 0; i < num_tags; i++) {
        // Apply scaling to tag coordinates
        int screen_x = (int)(tags[i].position.x * scale_factor + 320);
        int screen_y = (int)(240 - tags[i].position.y * scale_factor);  // Invert y to fit screen coordinates

        // Draw a larger dot for the tag
        SDL_RenderDrawPoint(renderer, screen_x, screen_y);
        SDL_Rect rect = {screen_x - 5, screen_y - 5, 10, 10};  // 10x10 rectangle for a bigger dot
        SDL_RenderFillRect(renderer, &rect);
    }
}

int main() {
    SDL_Window *window;
    SDL_Renderer *renderer;

    if (init_sdl(&window, &renderer) != 0) {
        return 1;
    }

    // Simulating some anchors (you can update with your real data)
    Point anchors[NUM_anchors] = {
        { -2.0f, -2.0f, 0.0f },
        {  2.0f, -2.0f, 0.0f },
        { -2.0f,  2.0f, 0.0f },
        {  2.0f,  2.0f, 0.0f }
    };

    Tag tags[MAX_TAGS];
    int num_tags = 0;

    int quit = 0;
    SDL_Event e;

    float scale_factor = 100.0f;  // Scale factor for adjusting coordinates

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        // Simulate parsing new coordinates
        if (parse_coordinates("teraterm.txt", tags, &num_tags) == 0) {
            printf("No more coordinates found for tags\n");
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black color for background
        SDL_RenderClear(renderer);

        // Draw anchors and tags
        draw_anchors(renderer, anchors, scale_factor);
        update_tags(renderer, tags, num_tags, scale_factor);

        // Present the drawn frame
        SDL_RenderPresent(renderer);

        // Delay for smooth animation
        SDL_Delay(50);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
