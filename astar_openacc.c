#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <openacc.h>

#define ROWS 150
#define COLS 150
#define SET_SIZE ROWS * COLS
#define MAX_FLOAT 1000000

typedef struct {
    int x, y;
} Vector2;

typedef struct {
    Vector2 pos;
    Vector2 parent;
    float costFromStart;
    float costToEnd;
} Node;

typedef struct {
    int hasValue;
    Node value;
} NodeNullable;

const Vector2 NULL_VECTOR = {-1, -1};
const Node NULL_NODE = {{-1, -1}, {-1, -1}, -1};

int equalVector2(Vector2 a, Vector2 b) {
    return a.x == b.x && a.y == b.y;
}

float cost(Node node) {
    return node.costFromStart + node.costToEnd;
}

#pragma region Set

typedef struct {
    Node data[SET_SIZE];
    int size;
} Set;

void init_set(Set* set) {
    set->size = 0;
}

void set(Set* set, Node element) {
    int index = -1;
    for (int i = 0; i < set->size; i++) {
        if (equalVector2(set->data[i].pos, element.pos)) {
            index = i;
        }
    }

    if (index != -1) {
        set->data[index] = element;
    } else {
        set->data[set->size++] = element;
    }
}

NodeNullable get_node_by_pos(Set* set, Vector2 pos) {
    for (int i = 0; i < set->size; i++) {
        if (equalVector2(set->data[i].pos, pos)) {
            Node node = set->data[i];
            return (NodeNullable){1, node};
        }
    }
    return (NodeNullable){0, NULL_NODE};
}

int contains(Set* set, Vector2 pos) {
    NodeNullable node = get_node_by_pos(set, pos);
    return node.hasValue;
}

NodeNullable pop_min_cost(Set* set) {
    if (set->size == 0) {
        return (NodeNullable){0, NULL_NODE};
    }

    int minIndex = 0;
    for (int i = 1; i < set->size; i++) {
        if (cost(set->data[i]) < cost(set->data[minIndex])) {
            minIndex = i;
        }
    }

    Node minCostElement = set->data[minIndex];

    for (int i = minIndex; i < set->size - 1; i++) {
        set->data[i] = set->data[i + 1];
    }
    set->size--;

    return (NodeNullable){1, minCostElement};
}

#pragma endregion


float distance(Vector2 a, Vector2 b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

int astar(Vector2 start, Vector2 end, int grid[ROWS][COLS]) {
    Set openSet; init_set(&openSet);
    Set closedSet; init_set(&closedSet);

    Node startNode = {start, NULL_VECTOR, 0, distance(start, end)};
    set(&openSet, startNode);

    NodeNullable endNode = {0, NULL_NODE};
    int endNodeFound = 0;

    #pragma acc parallel
    {
        //#pragma omp single
        {
            while (1) {
                int endNodeFoundLocal;
                int closedSetSize;
                #pragma acc critical
                {
                    endNodeFoundLocal = endNodeFound;
                    closedSetSize = closedSet.size;
                }

                if(endNodeFoundLocal || closedSetSize == SET_SIZE) {
                    break;
                }

                NodeNullable current;
                #pragma acc critical
                {
                    // Вытащить с минимальной ценой
                    current = pop_min_cost(&openSet);

                    if(current.hasValue) {
                        set(&closedSet, current.value);
                    }
                }

                if(!current.hasValue) {
                    //printf(" No cells - thread: %d\n", omp_get_thread_num());
                    continue;
                }

                //printf(" Take cell (%d,%d) - thread: %d\n", current.value.pos.x, current.value.pos.y, omp_get_thread_num());

                // Вернуть нод, если конечная точка.
                if (equalVector2(current.value.pos, end)) {
                    #pragma acc critical
                    {
                        endNode = (NodeNullable){1, current.value};
                        endNodeFound = 1;
                    }
                    break;
                }

                #pragma acc loop
                //{
                    for (int x = -1; x <= 1; x++) {
                        for (int y = -1; y <= 1; y++) {
                            if(abs(x) == abs(y)) {
                                continue;
                            }

                            Vector2 neighbourPos = {current.value.pos.x + x, current.value.pos.y + y};
                            
                            //printf("Neighbor for (%d,%d) - thread: %d\n", current.value.pos.x, current.value.pos.y, omp_get_thread_num());

                            int containsThis;
                            #pragma acc critical
                            {
                                containsThis = contains(&closedSet, neighbourPos);
                            }

                            if(containsThis) {
                                continue;
                            }

                            if (neighbourPos.x < 0 || neighbourPos.x >= ROWS || neighbourPos.y < 0 || neighbourPos.y >= COLS) {
                                continue;
                            }

                            int gridValue = grid[neighbourPos.x][neighbourPos.y];

                            if (gridValue == 0) {
                                continue;
                            }

                            NodeNullable neighbourNode;
                            #pragma acc critical
                            {
                                neighbourNode = get_node_by_pos(&openSet, neighbourPos);
                            }

                            float prevCostFromStart = neighbourNode.hasValue ? neighbourNode.value.costFromStart : MAX_FLOAT;
                            float prevCostToEnd = neighbourNode.hasValue ? neighbourNode.value.costToEnd : MAX_FLOAT;
                            float prevCost = prevCostFromStart + prevCostToEnd;

                            float costFromStart = current.value.costFromStart + gridValue;
                            float costToEnd = distance(neighbourPos, end);
                            float cost = costFromStart + costToEnd;

                            Node neighbour = {neighbourPos, current.value.pos, costFromStart, costToEnd};

                            if (cost < prevCost) {
                                #pragma acc critical
                                {
                                    set(&openSet, neighbour);
                                }
                            }
                        }
                    }
                //}
                #pragma acc wait
            }
        }
    }

    Vector2 path[SET_SIZE];
    int pathLength = 0;
    NodeNullable node = endNode;
    while (node.hasValue) {
        path[pathLength++] = node.value.pos;
        node = get_node_by_pos(&closedSet, node.value.parent);
    }

    for (int i = pathLength - 1; i >= 0; i--) {
        printf("(%d, %d) ", path[i].x, path[i].y);
    }

    printf("\n\n");
    for(int i = 0; i < ROWS; i++) {
        for(int j = 0; j < COLS; j++) {
            int inPath = 0;
            for (int k = 0; k < pathLength; k++) {
                if (path[k].x == i && path[k].y == j) {
                    inPath = 1;
                    break;
                }
            }
            if (inPath) {
                printf("X ");
            } else {
                printf("%d ", grid[i][j]);
            }
        }
        printf("\n");
    }
    
    return endNode.hasValue;
}

int main() {
    printf("Started\n");

    Vector2 start = {0, 0};
    Vector2 end = {ROWS - 1, COLS - 1};

    // int grid[ROWS][COLS] = {
    //     {1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    //     {0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    //     {0, 1, 1, 0, 1, 0, 0, 0, 0, 0},
    //     {0, 1, 0, 0, 1, 1, 1, 1, 0, 0},
    //     {0, 1, 0, 0, 0, 1, 0, 1, 0, 0},
    //     {1, 1, 0, 0, 1, 1, 0, 1, 0, 0},
    //     {1, 0, 0, 0, 1, 0, 0, 1, 0, 0},
    //     {1, 1, 1, 1, 1, 0, 0, 1, 0, 0},
    //     {0, 0, 0, 1, 0, 0, 0, 1, 0, 0},
    //     {0, 0, 0, 1, 1, 1, 1, 1, 1, 1}
    // };

    int grid[ROWS][COLS];

    for(int i = 0; i < ROWS; i++) {
        for(int j = 0; j < COLS; j++) {
            grid[i][j] = rand() % 5;
        }
    }

    int path_found = 0;

    // #pragma omp parallel
    // {
 
    //     printf("Hello World... from thread = %d\n", omp_get_thread_num());
    // }

    path_found = astar(start, end, grid);

    if (path_found) {
        printf("Путь найден!\n");
    } else {
        printf("Путь не найден!\n");
    }

    
    // #pragma omp parallel
    // {
    //     printf("A thread = %d\n", omp_get_thread_num());

    //     #pragma omp single
    //     {
    //         printf("B thread = %d\n", omp_get_thread_num());

    //         #pragma omp task
    //         {
    //             printf("C thread = %d\n", omp_get_thread_num());
    //         }
    //     } 
    // }

    return 0;
}