// Functions that allocate memory and that throw an error, release that memory themselves.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

size_t max_path_size = 0;

// TYPE DECLARATIONS //////////////////////////////////////////////////////////

typedef char * error_msg;

typedef unsigned int coordinate; // changing this requires to change the print_tile function!

typedef struct tile {
    coordinate x;
    coordinate y;

    struct tile * top_tile;
    struct tile * bot_tile;
    struct tile * right_tile;
    struct tile * left_tile;

    struct tile * matched_with;

    struct tile * visited_by;
} tile_t;

typedef struct tile_array {
    size_t size;
    tile_t * tiles;
} tile_array_t;

typedef struct tile_ptr_array {
    size_t size;
    tile_t ** tiles;
} tile_ptr_array_t;

// FUNCTION DECLARATIONS //////////////////////////////////////////////////////
 
void printc(char c);

void print_tile(tile_t * tile);

void print_tile_array(tile_array_t * tiles);

int comp_tiles(const void * void_tile1, const void * void_tile2);

int comp_tiles_eq(tile_t * tile1, tile_t * tile2);

#define DEBUG_APPEND_DIGIT
error_msg append_digit(coordinate * coord, char new_digit);

#define DEBUG_READ_LINE
error_msg read_line(tile_t * out_tile);

// #define DEBUG_CREATE_TILES
error_msg create_tiles(tile_array_t * out_tiles);

void link_tiles(tile_array_t * tiles);

tile_t * find_start_tile(tile_array_t * tiles);

error_msg build_reverse_path(tile_t * start, tile_t * end, tile_ptr_array_t * out_path);

// #define DEBUG_FIND_AGUMENTING_PATH
error_msg find_augmenting_path(tile_array_t * tiles, tile_ptr_array_t * out_path);
    
void assert_all_not_visited(tile_array_t * tiles);
    
// FUNCTION IMPLEMENTATIONS ///////////////////////////////////////////////////

void printc(char c) {
    // Only for debugging: prints a single character to stdout,
    // but replaces // special characters with a readable representation
    // 
    switch(c) {
        case ' ':  fprintf(stderr, "c = <SPACE>\n"); break;
        case '\n': fprintf(stderr, "c = <LINEFEED>\n"); break;
        case EOF:  fprintf(stderr, "c = <EOF>\n"); break;
        default:   fprintf(stderr, "c = %c\n", c);
    }
}


void print_tile(tile_t * tile){
    fprintf(stderr, "tile at: %p  x=%u y=%u  t=%p   b=%p   r=%p   l=%p  m=%p   v=%p  \n",
            (void*)tile, tile->x, tile->y,
            (void*)tile->top_tile, (void*)tile->bot_tile,
            (void*)tile->right_tile, (void*)tile->left_tile,
            (void*)tile->matched_with, (void*)tile->visited_by
            );
    // fprintf(stderr, "tile at: %p  x=%u y=%u  
// t=%p \tb=%p \tr=%p \tl=%p \t
// m=%p \tv=%p\t\n",
}


void print_tile_array(tile_array_t * tiles) {
    fprintf(stderr, "tile_array at: %p\n", (void*)tiles);
    fprintf(stderr, "  size=%lu\n", tiles->size);
    fprintf(stderr, "  tiles=\n");
    for( size_t cur_idx=0; cur_idx < tiles->size; ++cur_idx ) {
        fprintf(stderr, "    ");
        print_tile(&(tiles->tiles[cur_idx]));
    }
}


int comp_tiles(const void * void_tile1, const void * void_tile2) {
    // returns:  1, iff tile1 > tile2
    //           0, iff tile1 == tile2
    //          -1, iff tile1 < tile2

    tile_t * tile1 = (tile_t*) void_tile1;
    tile_t * tile2 = (tile_t*) void_tile2;

    if( tile1->x > tile2->x || (tile1->x == tile2->x && tile1->y > tile2->y) ) {
        return 1;
    } else if(tile1->x == tile2->x && tile1->y == tile2->y ) {
        return 0;
    }
    return -1;
}


int comp_tiles_eq(tile_t * tile1, tile_t * tile2) {
    // returns:  1, iff tile1 == tile2
    //           0, iff tile1 != tile2
    if( tile1->x == tile2->x && tile1->y == tile2->y ) {
        return 1;
    }
    return 0;
}


error_msg append_digit(coordinate * coord, char new_digit) {
    
    // increase significance of coord
    if( *coord <= UINT_MAX/10 ){
        *coord *= 10 ;
    } else {
        #ifdef DEBUG_APPEND_DIGIT
            fprintf(stderr, "coord = %u, new_digit = %c ", *coord, new_digit);
        #endif
        return "Err: Found coordinate greater than 2^32!\n";
    }
    
    // fill last digit with 
    coordinate new_digit_value = (new_digit - '0');
    if ( *coord <= UINT_MAX - new_digit_value ) {
        *coord += new_digit_value;
    } else {
        #ifdef DEBUG_APPEND_DIGIT
            fprintf(stderr, "coord = %u, new_digit = %c ", *coord/10, new_digit);
        #endif
        return "Err: Found coordinate greater than 2^32!\n";
    }
    
    return NULL;
    
}


error_msg read_line(tile_t * out_tile) {
    
    #ifdef DEBUG_READ_LINE
        static unsigned long line_count = 0;
        line_count ++;
    #endif
    
    char c = fgetc(stdin);
    if( feof(stdin) ){
        return NULL;
    }

    // skip leading <SPACE>s
    while( c == ' ' ) {
        c = fgetc(stdin);
    };

    // c is the first non <SPACE> char now
    // read x coordinate of tile number
    coordinate x = 0;
    do {
        if( isdigit(c) ) {
            error_msg error = append_digit(&x, c);
            if( error ){
                #ifdef DEBUG_READ_LINE
                    fprintf(stderr, "line %lu: ", line_count);
                #endif
                return error;
            }
        } else {
            
            #ifdef DEBUG_READ_LINE
                fprintf(stderr, "line %lu: ", line_count);printc(c);
            #endif
            
            return "Err: Found non-digit character in first coordinate of input line!\n";
        }
        c = fgetc(stdin);
    } while( c != ' ' );


    // c is a <SPACE> now
    // skip other <SPACE>s between the numbers
    while( c == ' ' ) {
        c = fgetc(stdin);
    };

    // c is a non <SPACE> char now
    coordinate y = 0;
    do {
        if( isdigit(c) ) {
            error_msg error = append_digit(&y, c);
            if( error ){
                #ifdef DEBUG_READ_LINE
                    fprintf(stderr, "line %lu: ", line_count);
                #endif
                return error;
            }
        } else {
            
            #ifdef DEBUG_READ_LINE
                fprintf(stderr, "line %lu: ", line_count);printc(c);
            #endif
            
            return "Err: Found non-digit character second coordinate of input line!\n";
        }
        c = fgetc(stdin);
    } while( c != ' '  && c != '\n' && c != EOF);

    while( c != '\n' && c != EOF ) {
        if( c != ' ' ) {
            
            #ifdef DEBUG_READ_LINE
                fprintf(stderr, "line %lu: ", line_count);printc(c);
            #endif
            
            return "Err: Found too many words in input line!\n";
        }
        c = fgetc(stdin);
    }

    out_tile->x = x;
    out_tile->y = y;
    out_tile->top_tile = NULL;
    out_tile->bot_tile = NULL;
    out_tile->right_tile = NULL;
    out_tile->left_tile = NULL;
    out_tile->matched_with = NULL;
    out_tile->visited_by = NULL;
    return NULL;
}


error_msg create_tiles(tile_array_t * out_tiles) {

    size_t max_num_tiles = 1<<10;
    size_t num_tiles = 0;

    tile_t * tiles = malloc(max_num_tiles*sizeof(tile_t));
    if( !tiles ){ return "Err: Out of memory!\n"; }

    while( !feof(stdin) ) {
        if( num_tiles == max_num_tiles ) {
            max_num_tiles *= 2;
            tiles = realloc(tiles, max_num_tiles*sizeof(tile_t));
            if( !tiles ){
                free(tiles);
                return "Err: Out of memory\n!";
            }
        }

        error_msg error = read_line(&tiles[num_tiles++]);
        if( error ) {
            free(tiles);
            return error;
        }
    }
    num_tiles--; // undo last increment

    if( num_tiles == 0 ) {
        out_tiles->tiles = tiles; // needs to be passed to get free correctly
        out_tiles->size = num_tiles;
        return NULL;
    }
    
    qsort((void*)tiles, num_tiles, sizeof(tile_t), comp_tiles);

    for( unsigned cur_tile = 0; cur_tile < num_tiles-1; cur_tile++  ) {
        if( comp_tiles_eq(&tiles[cur_tile], &tiles[cur_tile+1]) ) {
            
            #ifdef DEBUG_CREATE_TILES
                fprintf(stderr, "tile: "); print_tile(&tiles[cur_tile]);
            #endif

            free(tiles);
            
            return "Err: Found multiple given tile!\n";
        }
    }

    out_tiles->tiles = tiles;
    out_tiles->size  = num_tiles;
    return NULL;
}


void link_tiles(tile_array_t * tiles) {
    // precondition: tiles are sorted wrt. comp_tiles
 
    // size_t half_size = tiles->size/2;
    size_t size = tiles->size;
    
    for( size_t cur_idx=0; cur_idx < size-1; ++cur_idx ) {
        
        tile_t * cur_tile = &tiles->tiles[cur_idx];
        coordinate cur_x = cur_tile->x;
        coordinate cur_y = cur_tile->y;
        
        // === set top and bot links ===
        size_t nxt_idx = cur_idx + 1;
        tile_t * nxt_tile = &tiles->tiles[nxt_idx];
        
        coordinate nxt_x = nxt_tile->x;
        if( cur_x == nxt_x ) {
            coordinate nxt_y = nxt_tile->y;
            if( cur_y+1 == nxt_y ) {
                cur_tile->top_tile = nxt_tile;
                nxt_tile->bot_tile = cur_tile;
            }
        }

        // === set left and right links ===
        
        // skip tiles, that cannot have right links
        if( cur_x == UINT_MAX ) { continue; }
         
        // create a proxy tile to search for
        tile_t right_tile_proxy; 
        right_tile_proxy.x = cur_x + 1;
        right_tile_proxy.y = cur_y;
        
        tile_t * right_ptr = (tile_t*) bsearch((void*)&right_tile_proxy, (void*)cur_tile, 
                size - cur_idx , sizeof(tile_t), comp_tiles);

        if( right_ptr ) {
            cur_tile->right_tile = right_ptr;
            right_ptr->left_tile = cur_tile;
        }
        
    }
 
}


tile_t * find_start_tile(tile_array_t * tiles) {
    // find first start tile in tiles
     
    tile_t * start_tile = NULL;
    
    for( size_t cur_idx=0; cur_idx < tiles->size; ++cur_idx ) {
        
        tile_t * cur_tile = &tiles->tiles[cur_idx];
        
        if( !cur_tile->matched_with && !cur_tile->visited_by) {
            start_tile = cur_tile;
            break;
        }
    }

    return start_tile;
}


error_msg build_reverse_path(tile_t * start, tile_t * end, tile_ptr_array_t * out_path)  {
    // builds a path from end to start

    size_t max_path_length = 1 << 5;
    
    tile_t ** path = malloc(max_path_length*sizeof(tile_t*));
    if( !path ){ return "Err: Out of memory!\n"; }
    

    path[0] = end;
    size_t path_length = 1;
    tile_t * cur_tile = end;
    do {
        cur_tile = cur_tile->visited_by;
        
        if( path_length == max_path_length ) {
            max_path_length *= 2;
            path = realloc(path, max_path_length*sizeof(tile_t*));
            if( !path ){
                free(path);
                return "Err: Out of memory!\n";
            }
        }
        path[path_length++] = cur_tile;
    } while( cur_tile != start ); 

    out_path->tiles = path;
    out_path->size = path_length;

    // if( path_length == 34  ){
    //     for( size_t cur_idx = 0; cur_idx < path_length; cur_idx++ ) {
    //         print_tile(path[cur_idx]);
    //     }
    // fprintf(stderr, "there");
    // }

    return NULL;
}


error_msg find_augmenting_path(tile_array_t * tiles, tile_ptr_array_t * out_path) {
    // finds an augmenting path in tiles and stores it in out_path
    // if no such path exists out_path->size = 0
    
    
    out_path->size = 0;

    // initialize the search queue used for BFS
    size_t queue_capacity = 1 << 5;
    tile_t ** queue = malloc(queue_capacity*sizeof(tile_t*));
    if( !queue ){ return "Err: Out of memory!\n"; }

    // starting point of path
    tile_t * start = find_start_tile(tiles);
    if( !start ){ 
        free(queue);
        return NULL; 
    };
    queue[0] = start;

    #ifdef DEBUG_FIND_AGUMENTING_PATH
        fprintf(stderr, "  start_tile: ");
        print_tile(start);
    #endif
    
    tile_t * end = NULL;
    
    size_t pop_idx = 0;
    size_t push_idx = 1;
    while( pop_idx < push_idx && !end) {

        tile_t * cur_tile = queue[pop_idx++];
        tile_t * cur_children[] = {cur_tile->top_tile, cur_tile->bot_tile, cur_tile->right_tile, cur_tile->left_tile};

        for( size_t child_idx = 0; child_idx < 4; child_idx++ ) {
            
            tile_t * cur_child = cur_children[child_idx];
            
            if( cur_child && !cur_child->visited_by ) { // this child exists and has not been visited

                cur_child->visited_by = cur_tile;
                
                tile_t * match = cur_child->matched_with;
                if( match && !match->visited_by ) { // match exists and has not been visited

                    match->visited_by = cur_child;

                    if( push_idx == queue_capacity ) { // expand queue_capacity if needed
                        queue_capacity *= 2;
                        if( !(queue = realloc(queue, queue_capacity*sizeof(tile_t))) ) {
                            free(queue);
                            return "Err: Out of memory!\n";
                        }
                    }

                    queue[push_idx++] = match;

                } else { // match does not exist, so chils is a free tile
                    end = cur_child;
                    break;
                }
                
            }
 
        }
 
    }

    if( !end ) {
        free(queue);
        out_path->size = 1;
        return NULL;
    }
    
    size_t queue_length = push_idx;

    #ifdef DEBUG_FIND_AGUMENTING_PATH
        fprintf(stderr, "done building queue:\n");
        fprintf(stderr, "  queue_length: %lu\n", queue_length);
        for( size_t cur_idx = 0; cur_idx < queue_length; cur_idx++ ) {
            fprintf(stderr, "      ");
            print_tile(queue[cur_idx]);
        }
        fprintf(stderr, "  end:  ");
        print_tile(end);
        fprintf(stderr, "\n");
    #endif
    
    // build path -- traverse the visited_by from end start
    tile_ptr_array_t path;
    error_msg error = build_reverse_path(start, end, &path);
    if( error ){ return error; }
    
    #ifdef DEBUG_FIND_AGUMENTING_PATH
        fprintf(stderr, "done building path:\n");
        fprintf(stderr, "  path.size: %lu\n", path.size);
        for( size_t cur_idx = 0; cur_idx < path.size; cur_idx++ ) {
            fprintf(stderr, "      ");
            print_tile(path.tiles[cur_idx]);
        }
        fprintf(stderr, "\n");
    #endif
    
    // reset visited_by for all in queue
    for( size_t cur_idx = 0; cur_idx < queue_length; cur_idx++) {
        queue[cur_idx]->visited_by = NULL;
        if( queue[cur_idx]->matched_with ) {
            queue[cur_idx]->matched_with->visited_by = NULL;
        }
    }
    free(queue);
    
    #ifdef DEBUG_FIND_AGUMENTING_PATH
        fprintf(stderr, "done resetting queue:\n");
        fprintf(stderr, "  path.size: %lu\n", path.size);
        for( size_t cur_idx = 0; cur_idx < path.size; cur_idx++ ) {
            fprintf(stderr, "      ");
            print_tile(path.tiles[cur_idx]);
        }
        fprintf(stderr, "\n");
        print_tile_array(tiles);
    #endif
    
    // rereset visited_by for all in path
    for( size_t cur_idx = 0; cur_idx < path.size-1; cur_idx++) {
        path.tiles[cur_idx]->visited_by = path.tiles[cur_idx + 1];
    }
    path.tiles[path.size-1]->visited_by = path.tiles[0];
    
    #ifdef DEBUG_FIND_AGUMENTING_PATH
        fprintf(stderr, "  path.size: %lu\n", path.size);
        for( size_t cur_idx = 0; cur_idx < path.size; cur_idx++ ) {
            fprintf(stderr, "      ");
            print_tile(path.tiles[cur_idx]);
        }
        fprintf(stderr, "\n");
        print_tile_array(tiles);
    #endif
    
    out_path->tiles = path.tiles;
    out_path->size = path.size;
    return NULL;
}


void update_matching(tile_ptr_array_t * path) {
    for( size_t cur_idx = 0; cur_idx < path->size-1; cur_idx++ ) {
        tile_t * cur_tile = path->tiles[cur_idx];
        
        if( cur_idx % 2 == 0 ) {
            
            cur_tile->matched_with = cur_tile->visited_by;
            cur_tile->matched_with->matched_with = cur_tile;
            cur_tile->visited_by = NULL;
            
        }
        
        cur_tile->visited_by = NULL;
        
    }

    // remove visited_by flag from start
    path->tiles[path->size-1]->visited_by = NULL;
}


void print_matching(tile_array_t * tiles) {
    for( size_t cur_idx = 0; cur_idx < tiles->size; cur_idx++ ) {
        tile_t * cur_tile = &tiles->tiles[cur_idx];
        if( cur_tile->matched_with ) {
            tile_t * match = cur_tile->matched_with;
            printf("%u %u;%u %u\n",
                    cur_tile->x, cur_tile->y,
                    match->x, match->y);
            cur_tile->matched_with = NULL;
            match->matched_with = NULL;
        }
    }
}


void assert_all_not_visited(tile_array_t * tiles) {
    for( size_t cur_idx = 0; cur_idx < tiles->size; cur_idx++ ) {
        tile_t * cur_tile = &tiles->tiles[cur_idx];
        if( cur_tile->visited_by ) {
            fprintf(stderr, "THERE IS A VISITED TILE!!!\n");
            print_tile(cur_tile);
            exit(1);
        }
    }
}

// MAIN METHOD ////////////////////////////////////////////////////////////////
int main() {

    tile_array_t tiles;
    error_msg error;

    error = create_tiles(&tiles);
    if( error ) {
        fprintf(stderr, "%s", error);
        exit(1);
    } else if( tiles.size == 0 ) {
        free(tiles.tiles);
        return 0;
    } else if( tiles.size % 2 == 1 ){
        printf("None\n");
        free(tiles.tiles);
        return 0;
    }

    
    link_tiles(&tiles);

    while( 1 ) {
        
        tile_ptr_array_t path;
        error = find_augmenting_path(&tiles, &path);

        if( error ) {
            free(tiles.tiles);
            exit(1);
        }
        
        if( path.size == 0 ) {
            print_matching(&tiles);
            break;
        } else if (path.size == 1) {
            printf("None\n");
            break;
        }
        
        update_matching(&path);
        free(path.tiles);
    }
    
    free(tiles.tiles);
}
