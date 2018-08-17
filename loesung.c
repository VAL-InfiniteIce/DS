#include <stdlib.h>
#include <stdio.h>

typedef char* err;
union errData_u{
    int i;
    char c;
} errData;

typedef struct point_s{
    unsigned int x;
    unsigned int y;
} point_t;

typedef struct tile_s{
    struct tile_s * parent;
    unsigned int depth;
    struct tile_s * edge;

    struct tile_s * north;
    struct tile_s * west;
    struct tile_s * south;
    struct tile_s * east;

    point_t p;
} tile_t;

typedef struct allTiles_s{
    unsigned int amount;
   tile_t * tiles;
} allTiles_t;

err errMsg = NULL;

const char wrongChar[]  = "'%c' is an unallowed character!\n";
const char exceedMax[]  = "At least 1 coordinate is >2^32!\n";
const char wrongCoor[]  = "Line %i does not contain exact 2 arguments!\n";
const char doubleLine[] = "At least 2 lines containing the same tile!\n";
const char exceedMem[]  = "Not enough memory available!\n";
const char none[]       = "None\n";

void readStdIn(allTiles_t* allTiles);
void sort(tile_t * tiles, unsigned int begin, unsigned int end);
void link(allTiles_t* allTiles);
tile_t* search(allTiles_t * allTiles, unsigned int index, unsigned int findX, unsigned int findY);
int findCoverage(allTiles_t * allTiles);
int findAugmentedPath(allTiles_t * allTiles, tile_t* begin, tile_t*** p_tree, tile_t*** p_path);
void addNeighbours(tile_t* middle, tile_t** tree, unsigned int* index);
void resetTree(tile_t** tree);
void flipPath(tile_t** path);
void printResult(allTiles_t * allTiles);

int main()
{
    allTiles_t allTiles;
    allTiles.amount = 0;
    allTiles.tiles = (tile_t *) malloc(sizeof(tile_t));
    if (!allTiles.tiles)
    {
        errMsg = (err) exceedMem;
        goto err0;
    }

    /* Parsing Input TODO: (ausser letzte Zeile)
     * 
     * Fehler Zahl > 2^32
     * Fehler Char =/= ' ', '\t', 0123456789, '\n', '\r', "\r\n"
     * Fehler 0, 1 oder >2 Eintraegen tritt auf
     */
    readStdIn(&allTiles);
    if (errMsg) { goto err1; }
    if (allTiles.amount == 0) { goto err2; }
    
    /* Sort input and build structure
     *
     * Fehler 2 Gleiche Zeilen
     */
    sort(allTiles.tiles, 0, allTiles.amount-1);
    if (errMsg) { goto err1; }
    link(&allTiles);
    
    /* Check for augmented paths
     */
    int result = findCoverage(&allTiles);    // returns 1 if there are unconnectable knotes
    if (errMsg) { goto err1; }
    
    /* print result
     *
     * "None\n" falls nicht moeglich
     * "" falls leere Eingabe
     * "x_i y_i;x_j y_j" falls moeglich (2 benachbarte Kacheln)
     */
    if (result)
    {
        fprintf(stdout, none);
    } else {
        printResult(&allTiles);
    }

err2:
err1:
    free(allTiles.tiles);
err0:
    if (errMsg != NULL) { fprintf(stderr, errMsg, errData); }
    
    // */
    return errMsg != NULL;
}

void printResult(allTiles_t * allTiles)
{
    for (unsigned int i = 0; i < allTiles->amount; i++)
    {
        tile_t * current = &allTiles->tiles[i];
        if (!current->parent)
        {
            tile_t * other = current->edge;

            fprintf(stdout, "%u %u;%u %u\n", current->p.x, current->p.y, other->p.x, other->p.y);

            current->parent = current;
            other->parent = other;
        }
    }
    return;
}

void flipPath(tile_t** path)
{
    for (unsigned int i =  0; path[i]; i = i + 2)
    {
        path[i]->edge = path[i+1];
        path[i+1]->edge = path[i];
    }
    return;
}

void resetTree(tile_t** tree)
{
    for (unsigned int i = 0; tree[i]; i++)
    {
        tree[i]->parent = NULL;
    }
    return;
}

void addNeighbours(tile_t* middle, tile_t** tree, unsigned int* index)
{
    if (middle->north)
    {
        if (!middle->north->parent)
        {
            tree[*index] = middle->north;
            tree[*index]->depth = middle->depth +1;
            tree[(*index)++]->parent = middle;
        }
    }
    if (middle->west)
    {
        if (!middle->west->parent)
        {
            tree[*index] = middle->west;
            tree[*index]->depth = middle->depth +1;
            tree[(*index)++]->parent = middle;
        }
    }
    if (middle->south)
    {
        if (!middle->south->parent)
        {
            tree[*index] = middle->south;
            tree[*index]->depth = middle->depth +1;
            tree[(*index)++]->parent = middle;
        }
    }
    if (middle->east)
    {
        if (!middle->east->parent)
        {
            tree[*index] = middle->east;
            tree[*index]->depth = middle->depth +1;
            tree[(*index)++]->parent = middle;
        }
    }
    return;
}

int findAugmentedPath(allTiles_t * allTiles, tile_t* begin, tile_t*** p_tree, tile_t*** p_path)
{   
    tile_t** path = *p_path;
    // -1 Error
    // 0 Found Path
    // 1 No Path
    unsigned int length = 8;
    tile_t ** tree = (tile_t**) realloc(*p_tree, length * sizeof(tile_t*));
    if (!tree) { errMsg = (err) exceedMem; *p_tree[0] = NULL; return -1; }
    *p_tree = tree;

    unsigned int i = 0;
    unsigned int j = 1;
    int error = 1;
    tree[0] = begin;
    tree[0]->depth = 0;
    addNeighbours(tree[0], tree, &j);
    i++;
    do
    {
        if ( i >= j ) { error = 1; break; }
        if ( j >= length -4 )
        {
            length = length *2;
            if (length > allTiles->amount) { length = allTiles->amount; }
            tile_t ** temp1 = realloc(tree, length * sizeof(tile_t*));
            if (!temp1) { error = -1; break; }
            *p_tree = tree = temp1;
        }
        if (!tree[i]->edge)
        {
            error = 0;
            break;
        } else {
            if ( tree[i]->parent != tree[i]->edge )
            {
                tree[j] = tree[i]->edge;
                tree[j]->depth = tree[i]->depth +1;
                tree[j++]->parent = tree[i];
                i++;
                continue;
            }
        }
        addNeighbours(tree[i], tree, &j);
        i++;
    } while ( i < allTiles->amount );
    
    if (!error)
    {
        path[0] = tree[i];
        tile_t** temp2 = (tile_t**) realloc(path, (tree[i]->depth+2) * sizeof(tile_t*));
        if ( !temp2 )
        { 
            error = -1;
            errMsg = (err) exceedMem;
        } else{
    
            *p_path = path = temp2;
            unsigned int copyindex = 0;
            do
            {
                path[copyindex+1] = path[copyindex]->parent;
            } while (path[++copyindex]->parent);
            path[copyindex+1] = NULL;
            tree[j] = NULL;
        }
    } else {
        if (error == -1) { errMsg = (err) exceedMem; }
    }
    return error;
}

int findCoverage(allTiles_t * allTiles)
{
    int result = 0;
    for (unsigned int i = 0; i < allTiles->amount; i++)
    {
        if(allTiles->tiles[i].edge) { continue; }
        tile_t** tree = (tile_t**) malloc(sizeof(tile_t*));
        tile_t** path = (tile_t**) malloc(sizeof(tile_t*));

        if (!tree) { errMsg = (err) exceedMem; return -1; }
        if (!path) { errMsg = (err) exceedMem; free(tree); return -1; }

        result = findAugmentedPath(allTiles, &allTiles->tiles[i], &tree, &path);
        if( !result )
        {
            flipPath(path); 
            resetTree(tree);
        }

        free(path);
        free(tree);

        if (result < 0) { break; }
    }

    return result;
}

tile_t* search(allTiles_t * allTiles, unsigned int index, unsigned int findX, unsigned int findY)
{
    while (index < allTiles->amount)
    {
        if (allTiles->tiles[index].p.x < findX) { index++; continue; }
        if (allTiles->tiles[index].p.x == findX)
        {
            if (allTiles->tiles[index].p.y < findY) { index++; continue; }
            if (allTiles->tiles[index].p.y == findY)
            {
                return &allTiles->tiles[index];
            }
        }
        break;
    }

    return NULL;
}

void link(allTiles_t* allTiles)
{
    unsigned int i = 0;
    while ( i < allTiles->amount-1 )
    {
        tile_t * current = &allTiles->tiles[i];
        unsigned int cx = current->p.x;
        unsigned int cy = current->p.y;

        tile_t * north = search(allTiles, i, cx, cy + 1);
        if (north)
        {
            current->north = north;
            north->south = current;
            if (!current->edge && !north->edge)
            {
                current->edge = north;
                north->edge = current;
            }
        }
        tile_t * east = search(allTiles, i, cx + 1, cy);
        if (east)
        {
            current->east = east;
            east->west = current;
            if (!current->edge && !east->edge)
            {
                current->edge = east;
                east->edge = current;
            }
        }
        i++;
    }
    return; 
}

void sort(tile_t * tiles, unsigned int begin, unsigned int end)
{
    if ( (end - begin) == 0) { return; }

    unsigned int mid = (begin + end)/2;
    sort(tiles, begin, mid);
    if (errMsg) { return; }
    sort(tiles, mid+1, end);
    if (errMsg) { return; }

    point_t holder[end-begin +1];
    unsigned int i = begin;
    unsigned int j = mid +1;
    unsigned int k = 0;

    while (i <= mid && j <= end)
    {
        if (tiles[i].p.x < tiles[j].p.x)
        {
            holder[k].x = tiles[i].p.x;
            holder[k].y = tiles[i].p.y;
            k++;
            i++;
            continue;
        }
        if (tiles[i].p.x > tiles[j].p.x)
        {
            holder[k].x = tiles[j].p.x;
            holder[k].y = tiles[j].p.y;
            k++;
            j++;
            continue;
        }
        if (tiles[i].p.y < tiles[j].p.y)
        {
            holder[k].x = tiles[i].p.x;
            holder[k].y = tiles[i].p.y;
            k++;
            i++;
            continue;
        } else 
        if (tiles[i].p.y > tiles[j].p.y)
        {
            holder[k].x = tiles[j].p.x;
            holder[k].y = tiles[j].p.y;
            k++;
            j++;
            continue;
        }
        errMsg = (err) doubleLine;
        return;
    }
    while ( i <= mid )
    {
        holder[k].x = tiles[i].p.x;
        holder[k].y = tiles[i].p.y;
        k++;
        i++;
    }
    
    i = begin;
    k = 0;
    while ( i < j )
    {
        tiles[i].p.x = holder[k].x;
        tiles[i].p.y = holder[k].y;
        i++;
        k++;
    }
    return;
}

void readStdIn(allTiles_t* allTiles)
{
    char c;
    int i = 0;
    // File
    while ( (c = getchar()) != EOF)
    {
        i++;
        // Line
        unsigned long a = 0;
        unsigned long b = 0;

        // ' '*
        while ( c == ' ' || c == '\t' ) { c = getchar(); }
        
        if (c == '\n' || c == '\r')
        {
            printf("HERE!");
            errMsg = (err) wrongCoor;
            errData.i = i;
            return;
        }
        // 0-9+
        if (c >= '0' && c <= '9')
        {
            do
            {
                a = a*10 + (unsigned long) c -48; 
                if ( a >= 4294967296 )
                {
                    errMsg = (err) exceedMax;
                    return;
                }
            } while ( (c = getchar()) >= '0' && c <= '9');
        }
        else
        {
            errMsg = (err) wrongChar;
            errData.c = c;
            return;
        }

        // ' '+
        if (c == '\n' || c == '\r')
        {
            errMsg = (err) wrongCoor;
            errData.i = i;
            return;
        }
        if ( c == ' ' || c == '\t')
        {
            do { c = getchar(); } while( c == ' ' || c == '\t' );
        }
        else
        {
            errMsg = (err) wrongChar;
            errData.c = c;
            return;
        }

        if (c == '\n' || c == '\r')
        {
            errMsg = (err) wrongCoor;
            errData.i = i;
            return;
        }
        // 0-9+
        if (c >= '0' && c <= '9')
        {
            do
            {
                b = b*10 + (unsigned long) c -48; 
                if ( b >= 4294967296 )
                {
                    errMsg = (err) exceedMax;
                    return;
                }
            } while ( (c = getchar()) >= '0' && c <= '9');
        }
        else
        {
            errMsg = (err) wrongChar;
            errData.c = c;
            return;
        }

        // ' '*
        while (c == ' ' || c == '\t') { c = getchar(); }

        allTiles->amount++;
        tile_t* temp = (tile_t*) realloc(allTiles->tiles, allTiles->amount*sizeof(tile_t));
        if (!temp)
        {
            errMsg = (err) exceedMem;
            return;
        }

        temp[allTiles->amount-1].p.x = (unsigned int) a;
        temp[allTiles->amount-1].p.y = (unsigned int) b;
        temp[allTiles->amount-1].edge= NULL;
        temp[allTiles->amount-1].parent = NULL;
        temp[allTiles->amount-1].north = NULL;
        temp[allTiles->amount-1].south = NULL;
        temp[allTiles->amount-1].west = NULL;
        temp[allTiles->amount-1].east = NULL;

        allTiles->tiles = temp;
        
        if ( c == EOF) {
            printf("TEST!\n");
            break;
        }
        if ( c != '\n' && c != '\r')
        {
            errMsg = (err) wrongCoor;
            errData.i = i;
            return;
        }
    }
    return;
}

