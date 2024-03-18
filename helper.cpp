#include <cmath>
#include <vector>


class Block{
    public:
        int min_addr;
        int max_addr;
        bool dirty;
        bool valid;

        Block(){
            min_addr = -1;
            max_addr = -1;
            dirty = false;
            valid = false;
        }

        ~Block(){}
};

class Level{
public:
    int num_rows;
    int ways;
    double misses;

    Level(){
        num_rows = 0;
        ways = 0;
        misses = 0;
    }

    ~Level(){}
};

std::vector<std::vector<Block> > L1;
std::vector<std::vector<Block> > L2;
std::vector<std::vector<int> > LRU1;
std::vector<std::vector<int> > LRU2;
Level level1;
Level level2;
double total_time = 0;
double attempts_L2 = 0;

void init_level(int assoc, int size, int b_size, int level){
    int ways = std::pow(2, assoc);
    int cache_size = std::pow(2, size);
    int block_size = std::pow(2, b_size);
    int num_rows = cache_size / ways;
    num_rows = num_rows / block_size;
    Block block0;
    std::vector<Block> way0;

    // Setting ways vector
    way0.reserve(num_rows);
    for(int j = 0; j < num_rows; j++){
        way0.push_back(block0);
    }

    // Setting the LRU for a single set
    std::vector<int> lru0;
    lru0.reserve(ways);
    // Setting default LRU
    for(int j = 0; j < ways; j++){
        lru0.push_back(j);
    }

    if(level == 1){
        level1.num_rows = num_rows;
        level1.ways = ways;
        // Setting L1
        L1.reserve(ways);
        for(int i = 0; i < ways; i++){
            L1.push_back(way0);
        }

        // Setting L1 LRU
        LRU1.reserve(num_rows);
        for(int i = 0; i < num_rows; i++){
            LRU1.push_back(lru0);
        }

    }
    else if(level == 2){
        level2.num_rows = num_rows;
        level2.ways = ways;
        // Setting L2
        L2.reserve(ways);
        for(int i = 0; i < ways; i++){
            L2.push_back(way0);
        }

        // Setting L2 LRU
        LRU2.reserve(num_rows);
        for(int i = 0; i < num_rows; i++){
            LRU2.push_back(lru0);
        }
    }

    // cleaning memory
    std::vector<int>::iterator it = lru0.begin();
    while(it != lru0.end()){
        lru0.erase(it);
    }
    std::vector<Block>::iterator itb = way0.begin();
    while(itb != way0.end()){
        way0.erase(itb);
    }
}

int search_in_level(int address, std::vector<std::vector<Block> > L, int BSize, Level level){
    int set = address >> BSize;
    set = set & int(level.num_rows - 1);
    for(int i = 0; i < level.ways; i++){
        if(L[i][set].min_addr <= address && L[i][set].max_addr >= address && L[i][set].valid){
            return i;
        }
    }
    return -1;
}

void update_LRU(int level, int way, int set){
    if(level == 1){
        int x = LRU1[set][way];
        LRU1[set][way] = level1.ways - 1;
        for(int j = 0; j < level1.ways; j++){
            if(j != way && LRU1[set][j] > x){
                LRU1[set][j]--;
            }
        }
    }
    else if(level == 2){
        int x = LRU2[set][way];
        LRU2[set][way] = level2.ways - 1;
        for(int j = 0; j < level2.ways; j++){
            if(j != way && LRU2[set][j] > x){
                LRU2[set][j]--;
            }
        }
    }
}

int search_victim(int set, int level){
    int index = 0;
    if(level == 1){
        for(int i = 0; i < level1.ways; i++){
            // Search for the LRU way in set
            if(LRU1[set][i] == 0){
                index = i;
                return index;
            }
        }
    }
    else if(level == 2){
        for(int i = 0; i < level2.ways; i++){
            // Search for the LRU way in set
            if(LRU2[set][i] == 0){
                index = i;
                return index;
            }
        }
    }
    return -1;
}


void read(int address, int BSize, int L1cyc, int L2cyc, int MEMcyc){
    total_time += L1cyc;
    // calc min and max address
    int offset = address & int(std::pow(2, BSize) -1);
    int min_address = address - offset;
    int max_address = min_address + int(std::pow(2, BSize) -1);
    int set1 = min_address >> BSize;
    set1 = set1 & int(level1.num_rows - 1);
    int set2 = min_address >> BSize;
    set2 = set2 & int(level2.num_rows - 1);


    int way1 = search_in_level(address, L1, BSize, level1);
    if(way1 == -1){
        // Miss L1
        total_time += L2cyc;
        level1.misses++;
        attempts_L2++;
        int way2 = search_in_level(address, L2, BSize, level2);
        if(way2 == -1){
            // Miss L1 and in L2
            total_time += MEMcyc;
            level2.misses++;
            int victim_way = search_victim(set2, 2);
            if(L2[victim_way][set2].valid){
                // need to evict from L2
                // if victim line in L2 is dirty -> write to mem. we neglected that.
                L2[victim_way][set2].valid = false;
                int vic_way1 = search_in_level(L2[victim_way][set2].min_addr, L1, BSize, level1);
                if(vic_way1 != -1){
                    // evicted block from L2 exists in L1
                    // if victim line in L1 is dirty -> write to mem. we neglected that.
                    int temp_set = L2[victim_way][set2].min_addr >> BSize;
                    temp_set = temp_set & int(level1.num_rows - 1);
                    L1[vic_way1][temp_set].valid = false;
                }
            }
            // insert new block to L2
            L2[victim_way][set2].min_addr = min_address;
            L2[victim_way][set2].max_addr = max_address;
            L2[victim_way][set2].valid = true;
            L2[victim_way][set2].dirty = false;
            update_LRU(2, victim_way, set2);

            // insert new block to L1
            int victim_way1 = search_victim(set1, 1);
            if(L1[victim_way1][set1].valid){
                // need to evict from L1
                if(L1[victim_way1][set1].dirty){
                    // update value in L2
                    int update2 = search_in_level(L1[victim_way1][set1].min_addr, L2, BSize, level2);
                    int temp_set2 = L1[victim_way1][set1].min_addr >> BSize;
                    temp_set2 = temp_set2 & int(level2.num_rows - 1);
                    L2[update2][temp_set2].dirty = true;
                    update_LRU(2, update2, temp_set2);
                }
            }
            // insert new block to L1
            L1[victim_way1][set1].min_addr = min_address;
            L1[victim_way1][set1].max_addr = max_address;
            L1[victim_way1][set1].valid = true;
            L1[victim_way1][set1].dirty = false;
            update_LRU(1, victim_way1, set1);
        }
        else{
            // Miss L1 Hit in L2
            update_LRU(2, way2, set2);

            // insert to L1
            int victim1 = search_victim(set1, 1);
            if(L1[victim1][set1].valid){
                // need to evict from L1
                if(L1[victim1][set1].dirty){
                    int update2 = search_in_level(L1[victim1][set1].min_addr, L2, BSize, level2);
                    int temp_set2 = L1[victim1][set1].min_addr >> BSize;
                    temp_set2 = temp_set2 & int(level2.num_rows - 1);
                    L2[update2][temp_set2].dirty = true;
                    update_LRU(2, update2, temp_set2);
                }
            }
            // insert new block to L1
            L1[victim1][set1].min_addr = min_address;
            L1[victim1][set1].max_addr = max_address;
            L1[victim1][set1].valid = true;
            L1[victim1][set1].dirty = false;
            update_LRU(1, victim1, set1);
        }
    }
    else{
        // Hit L1
        update_LRU(1, way1, set1);
    }
}


void write(int address, int BSize, int L1cyc, int L2cyc, int MEMcyc, int allocate){
    total_time += L1cyc;
    // calc min and max address
    int offset = address & int(std::pow(2, BSize) -1);
    int min_address = address - offset;
    int max_address = min_address + int(std::pow(2, BSize) -1);
    int set1 = min_address >> BSize;
    set1 = set1 & int(level1.num_rows - 1);
    int set2 = min_address >> BSize;
    set2 = set2 & int(level2.num_rows - 1);
    int way1 = search_in_level(address, L1, BSize, level1);
    if(way1 == -1){
        // Miss L1
        total_time += L2cyc;
        level1.misses++;
        attempts_L2++;
        int way2 = search_in_level(address, L2, BSize, level2);
        if(way2 == -1){
            // Miss L1 Miss L2
            total_time += MEMcyc;
            level2.misses++;

            if(allocate == 1){
                // write allocate
                int victim2 = search_victim(set2, 2);
                if(L2[victim2][set2].valid){
                    // need to evict from L2
                    // if victim line in L2 is dirty -> write to mem. we neglected that.
                    int snoop1 = search_in_level(L2[victim2][set2].min_addr, L1, BSize, level1);
                    int temp_set1 = L2[victim2][set2].min_addr >> BSize;
                    temp_set1 = temp_set1 & int(level1.num_rows - 1);
                    if(snoop1 != -1){
                        // exists in L1
                        L1[snoop1][temp_set1].valid = false;
                    }
                }
                // insert new block to L2
                L2[victim2][set2].min_addr = min_address;
                L2[victim2][set2].max_addr = max_address;
                L2[victim2][set2].valid = true;
                L2[victim2][set2].dirty = false;
                update_LRU(2, victim2, set2);

                // insert new block to L1
                int victim1 = search_victim(set1, 1);
                if(L1[victim1][set1].valid){
                    // need to evict from L1
                    if(L1[victim1][set1].dirty){
                        int update2 = search_in_level(L1[victim1][set1].min_addr, L2, BSize, level2);
                        int temp_set2 = L1[victim1][set1].min_addr >> BSize;
                        temp_set2 = temp_set2 & int(level2.num_rows - 1);
                        L2[update2][temp_set2].dirty = true;
                        update_LRU(2, update2, temp_set2);
                    }
                }
                // insert new block to L1
                L1[victim1][set1].min_addr = min_address;
                L1[victim1][set1].max_addr = max_address;
                L1[victim1][set1].valid = true;
                L1[victim1][set1].dirty = true;
                update_LRU(1, victim1, set1);
            }
            else{
                // write no allocate
                // write to mem. we neglected that.
            }
        }
        else {
            // Miss L1 Hit L2
            update_LRU(2, way2, set2);
            if (allocate == 1) {
                // write allocate
                // insert new block to L1
                int victim1 = search_victim(set1, 1);
                if (L1[victim1][set1].valid) {
                    // need to evict from L1
                    if (L1[victim1][set1].dirty) {
                        int update2 = search_in_level(L1[victim1][set1].min_addr, L2, BSize, level2);
                        int temp_set2 = L1[victim1][set1].min_addr >> BSize;
                        temp_set2 = temp_set2 & int(level2.num_rows - 1);
                        L2[update2][temp_set2].dirty = true;
                        update_LRU(2, update2, temp_set2);
                    }
                }
                // insert new block to L1
                L1[victim1][set1].min_addr = min_address;
                L1[victim1][set1].max_addr = max_address;
                L1[victim1][set1].valid = true;
                L1[victim1][set1].dirty = true;
                update_LRU(1, victim1, set1);
            } else {
                // write no allocate
                L2[way2][set2].dirty = true;
            }
        }
    }
    else{
        // L1 hit
        L1[way1][set1].dirty = true;
        update_LRU(1, way1, set1);
    }
}
