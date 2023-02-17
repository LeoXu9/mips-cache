/*************************************************************************************|
|   1. YOU ARE NOT ALLOWED TO SHARE/PUBLISH YOUR CODE (e.g., post on piazza or online)|
|   2. Fill memory_hierarchy.c                                                        |
|   3. Do not use any other .c files neither alter mipssim.h or parser.h              |
|   4. Do not include any other library files                                         |
|*************************************************************************************/

#include "mipssim.h"

uint32_t cache_type = 0;

//cache structure and its variables defined
struct cache{
    int *data;
    int valid_bit;
    int tag;
};

struct cache * pcache; //pointer pointing to the cache
int num_bits_index = 0;
int num_bits_tag = 0;
int byte_offset = 2;
int word_offset = 2;
int num_bits_addr = 32;
int tag;
int iniad;
int index_;
int num_blocks;
int j;

void memory_state_init(struct architectural_state* arch_state_ptr) {
    arch_state_ptr->memory = (uint32_t *) malloc(sizeof(uint32_t) * MEMORY_WORD_NUM);
    memset(arch_state_ptr->memory, 0, sizeof(uint32_t) * MEMORY_WORD_NUM);
    if (cache_size == 0) {
        // CACHE DISABLED
        memory_stats_init(arch_state_ptr, 0); // WARNING: we initialize for no cache 0
    } else {
        // CACHE ENABLED
        num_blocks = cache_size / 16; // 4 bytes in a word so divided by 4
        //int  = num_words / 4; // 4 words in a block so divided by 4
        switch(cache_type) {
        case CACHE_TYPE_DIRECT: // direct mapped
            // dynaminally build a cache array
            pcache = calloc(num_blocks, sizeof(struct cache));

            //dynaminally build data storage
            for(int i=0; i< num_blocks; i++)
            {
                pcache[i].data = calloc(4, sizeof(WORD_SIZE));
            }
            // find the number of bits that the index needs
            while(num_blocks >= 2){
                num_blocks = num_blocks / 2;
                num_bits_index++;
            }
            num_bits_tag = num_bits_addr - num_bits_index - word_offset - byte_offset;
            //printf("num_bits_tag ================= %d", num_bits_tag);
            memory_stats_init(arch_state_ptr, num_bits_tag);
            break;
        case CACHE_TYPE_FULLY_ASSOC: // fully associative
            // dynaminally build a cache array
            pcache = calloc(num_blocks, sizeof(struct cache));

            // dynaminally build data storage
            for(int i=0; i< num_blocks; i++)
            {
                pcache[i].data = calloc(4, sizeof(WORD_SIZE));
            }
            // find the number of bits that the index needs
            while(num_blocks >= 2){
                num_blocks = num_blocks / 2;
                num_bits_index++;
            }
            num_bits_tag = num_bits_addr - num_bits_index - word_offset - byte_offset;
            //printf("num_bits_tag ================= %d", num_bits_tag);
            memory_stats_init(arch_state_ptr, num_bits_tag);
            break;
        case CACHE_TYPE_2_WAY: // 2-way associative
            // dynaminally build a cache array
            pcache = calloc(num_blocks, sizeof(struct cache));

            // dynaminally build data storage
            for(int i=0; i< num_blocks; i++)
            {
                pcache[i].data = calloc(4, sizeof(WORD_SIZE));
            }
            // find the number of bits that the index needs
            while(num_blocks >= 2){
                num_blocks = num_blocks / 2;
                num_bits_index++;
            }
            num_bits_tag = num_bits_addr - num_bits_index - 2 - byte_offset;
            //printf("num_bits_tag ================= %d", num_bits_tag);
            memory_stats_init(arch_state_ptr, num_bits_tag);
            break;
        }
    }
}

// returns data on memory[address / 4]
int memory_read(int address){
    arch_state.mem_stats.lw_total++;
    check_address_is_word_aligned(address);

    if (cache_size == 0) {
        // CACHE DISABLED
        return (int) arch_state.memory[address / 4];
    } else {
        // CACHE ENABLED
        //assert(0); /// @students: Remove assert(0); and implement Memory hierarchy w/ cache
        
        /// @students: your implementation must properly increment: arch_state_ptr->mem_stats.lw_cache_hits
        
        switch(cache_type) {
        case CACHE_TYPE_DIRECT: // direct mapped
            word_offset = get_piece_of_a_word(address, 0, 4);
            index_ = get_piece_of_a_word(address, 4, num_bits_index);
            tag = get_piece_of_a_word(address, 4 + num_bits_index, num_bits_tag);
            if((pcache[index_].valid_bit) && (tag == pcache[index_].tag))
            // if valid bit is 1 and if the tag in the address is equal to the tag in the cache
                {arch_state.mem_stats.lw_cache_hits++;//increment cache hit
                //printf("%lf", arch_state.mem_stats.lw_cache_hits);
                return (int) pcache[index_].data[word_offset / 4];// return 
                }

            // if valid bit is 1 and the tags are not the same
            else if((pcache[index_].valid_bit) && (tag != pcache[index_].tag))
                {pcache[index_].tag = tag; // update tag
                iniad = address / 4 - word_offset / 4; // fetch the data from this address
                for(int i = 0; i < 4; i++){ // fetch 4 words
                    pcache[index_].data[i] = arch_state.memory[iniad+i];
                }
                return (int) pcache[index_].data[word_offset / 4]; // return the specific word
                }

            //if valid bit is 0
            else{
                pcache[index_].valid_bit = 1; // valid bit 0 to 1
                pcache[index_].tag = tag; // update tag
                iniad = address / 4 - word_offset / 4; // fetch the data from this address 
                for(int i = 0; i < 4; i++){ // fetch 4 words
                    pcache[index_].data[i] = arch_state.memory[iniad+i]; // return the specific word
                    return (int) pcache[index_].data[word_offset / 4]; // return the specific word
                    }
                }
            break;


        case CACHE_TYPE_FULLY_ASSOC: // fully associative
            word_offset = get_piece_of_a_word(address, 0, 4);
            index_ = get_piece_of_a_word(address, 4, num_bits_index);
            tag = get_piece_of_a_word(address, 4 + num_bits_index, num_bits_tag);
            if(pcache[index_].valid_bit && tag == pcache[index_].tag)
            // if valid bit is 1 and if the tag in the address is equal to the tag in the cache
                { arch_state.mem_stats.lw_cache_hits++;//increment cache hit
                //printf("%lf", arch_state.mem_stats.lw_cache_hits);
                return (int) pcache[index_].data[word_offset / 4];// return 
                }

            // if valid bit is 1 and the tags are not the same
            else if(pcache[index_].valid_bit == 1 && tag != pcache[index_].tag)
                {pcache[index_].tag = tag; // update tag
                iniad = address / 4 - word_offset / 4; // fetch the data from this address
                for(int i = 0; i < 4; i++){ // fetch 4 words
                    pcache[index_].data[i] = arch_state.memory[iniad + i];
                }
                return (int) pcache[index_].data[word_offset / 4]; // return the specific word
                }

            //if valid bit is 0
            else{
                pcache[index_].valid_bit = 1; // valid bit 0 to 1
                pcache[index_].tag = tag; // update tag
                iniad = address / 4 - word_offset / 4; // fetch the data from this address 
                for(int i = 0; i < 4; i++){ // fetch 4 words
                    pcache[index_].data[i] = arch_state.memory[iniad + i]; // return the specific word
                    return (int) pcache[index_].data[word_offset / 4]; // return the specific word
                    }
                }
            break;


        case CACHE_TYPE_2_WAY: // 2-way associative
            word_offset = get_piece_of_a_word(address, 0, 4);
            index_ = get_piece_of_a_word(address, 4, num_bits_index);
            tag = get_piece_of_a_word(address, 4 + num_bits_index, num_bits_tag);
            if(pcache[index_].valid_bit && tag == pcache[index_].tag)
            // if valid bit is 1 and if the tag in the address is equal to the tag in the cache
                { arch_state.mem_stats.lw_cache_hits++;//increment cache hit
                //printf("%lf", arch_state.mem_stats.lw_cache_hits);
                return (int) pcache[index_].data[word_offset / 4];// return 
                }

            // if valid bit is 1 and the tags are not the same
            else if(pcache[index_].valid_bit == 1 && tag != pcache[index_].tag)
                {pcache[index_].tag = tag; // update tag
                iniad = address / 4 - word_offset / 4; // fetch the data from this address
                for(int i = 0; i < 4; i++){ // fetch 4 words
                    pcache[index_].data[i] = arch_state.memory[iniad + i];
                }
                return (int) pcache[index_].data[word_offset / 4]; // return the specific word
                }

            //if valid bit is 0
            else{
                pcache[index_].valid_bit = 1; // valid bit 0 to 1
                pcache[index_].tag = tag; // update tag
                iniad = address / 4 - word_offset / 4; // fetch the data from this address 
                for(int i = 0; i < 4; i++){ // fetch 4 words
                    pcache[index_].data[i] = arch_state.memory[iniad + i]; // return the specific word
                    return (int) pcache[index_].data[word_offset / 4]; // return the specific word
                    }
                }
            break;
        }
    }
    return 0;
}

// writes data on memory[address / 4]
void memory_write(int address, int write_data) {
    arch_state.mem_stats.sw_total++;
    check_address_is_word_aligned(address);

    if (cache_size == 0) {
        // CACHE DISABLED
        arch_state.memory[address / 4] = (uint32_t) write_data;
    } else {
        // CACHE ENABLED
        //assert(0); /// @students: Remove assert(0); and implement Memory hierarchy w/ cache
        
        /// @students: your implementation must properly increment: arch_state_ptr->mem_stats.sw_cache_hits
        
        switch(cache_type) {
        case CACHE_TYPE_DIRECT: // direct mapped
            word_offset = get_piece_of_a_word(address, 0, 4); // last 4 bits as the offset
            index_ = get_piece_of_a_word(address, 4, num_bits_index); // index bits
            tag = get_piece_of_a_word(address, 4 + num_bits_index, num_bits_tag); // tag bits

            // when it's a write hit
            if(pcache[index_].valid_bit){ // if the valid bit is 1
                if(tag == pcache[index_].tag){ // and the tags are equal
                    arch_state.mem_stats.sw_cache_hits++; // increment cache hit
                    //printf("%lf", arch_state.mem_stats.sw_cache_hits);
                    pcache[index_].data[word_offset / 4] = (uint32_t)write_data; // write the data to cache
                    arch_state.memory[address / 4] = (uint32_t)write_data; // write the data to memory as well
                }
            }
            else{
                //when it's a write miss
                arch_state.memory[address / 4] = (uint32_t) write_data;
            }

            
            break;
        case CACHE_TYPE_FULLY_ASSOC: // fully associative
            word_offset = get_piece_of_a_word(address, 0, 4); // last 4 bits as the offset
            index_ = get_piece_of_a_word(address, 4, num_bits_index); // index bits
            tag = get_piece_of_a_word(address, 4 + num_bits_index, num_bits_tag); // tag bits

            // when it's a write hit
            if(pcache[index_].valid_bit){ // if the valid bit is 1
                if(tag == pcache[index_].tag){ // and the tags are equal
                    arch_state.mem_stats.sw_cache_hits++; // increment cache hit
                    //printf("%lf", arch_state.mem_stats.sw_cache_hits);
                    pcache[index_].data[word_offset  / 4] = write_data; // write the data to cache
                    arch_state.memory[address / 4] = write_data; // write the data to memory as well
                }
            }

            //when it's a write miss
            arch_state.memory[address / 4] = (uint32_t) write_data;
            break;
        case CACHE_TYPE_2_WAY: // 2-way associative
            word_offset = get_piece_of_a_word(address, 0, 4); // last 4 bits as the offset
            index_ = get_piece_of_a_word(address, 4, num_bits_index); // index bits
            tag = get_piece_of_a_word(address, 4 + num_bits_index, num_bits_tag); // tag bits

            // when it's a write hit
            if(pcache[index_].valid_bit){ // if the valid bit is 1
                if(tag == pcache[index_].tag){ // and the tags are equal
                    arch_state.mem_stats.sw_cache_hits++; // increment cache hit
                    //printf("%" PRIu64 "\n", arch_state.mem_stats.sw_cache_hits);
                    pcache[index_].data[word_offset / 4] = write_data; // write the data to cache
                    arch_state.memory[address/4] = write_data; // write the data to memory as well
                }
            }

            //when it's a write miss
            arch_state.memory[address / 4] = (uint32_t) write_data;
            break;
        }
    }
}
