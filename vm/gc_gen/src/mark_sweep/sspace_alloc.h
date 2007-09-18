/*
 *  Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef _SSPACE_ALLOC_H_
#define _SSPACE_ALLOC_H_

#include "sspace_chunk.h"
#include "sspace_mark_sweep.h"
#include "../common/gc_concurrent.h"
#include "../common/collection_scheduler.h"

extern POINTER_SIZE_INT cur_alloc_color;
extern POINTER_SIZE_INT cur_alloc_mask;
extern POINTER_SIZE_INT cur_mark_mask;


inline Boolean slot_is_alloc_in_table(POINTER_SIZE_INT *table, unsigned int slot_index)
{
  unsigned int color_bits_index = slot_index * COLOR_BITS_PER_OBJ;
  unsigned int word_index = color_bits_index / BITS_PER_WORD;
  unsigned int index_in_word = color_bits_index % BITS_PER_WORD;
  
  return (Boolean)(table[word_index] & (cur_alloc_color << index_in_word));
}

#ifdef _DEBUG
static Boolean slot_is_free_in_table(POINTER_SIZE_INT *table, unsigned int slot_index)
{
  assert(!slot_is_alloc_in_table(table, slot_index));
  
  unsigned int color_bits_index = slot_index * COLOR_BITS_PER_OBJ;
  unsigned int word_index = color_bits_index / BITS_PER_WORD;
  unsigned int index_in_word = color_bits_index % BITS_PER_WORD;
  
  return !(table[word_index] & cur_alloc_color << index_in_word);
  
}
#endif

inline unsigned int composed_slot_index(unsigned int word_index, unsigned int index_in_word)
{
  unsigned int color_bits_index = word_index*BITS_PER_WORD + index_in_word;
  return color_bits_index/COLOR_BITS_PER_OBJ;
}

inline unsigned int next_free_index_in_color_word(POINTER_SIZE_INT word, unsigned int index)
{
  while(index < BITS_PER_WORD){
    if(!(word & (cur_alloc_color << index)))
      return index;
    index += COLOR_BITS_PER_OBJ;
  }
  return MAX_SLOT_INDEX;
}

inline unsigned int next_alloc_index_in_color_word(POINTER_SIZE_INT word, unsigned int index)
{
  while(index < BITS_PER_WORD){
    if(word & (cur_alloc_color << index))
      return index;
    index += COLOR_BITS_PER_OBJ;
  }
  return MAX_SLOT_INDEX;
}

inline unsigned int next_free_slot_index_in_table(POINTER_SIZE_INT *table, unsigned int slot_index, unsigned int slot_num)
{
  assert(slot_is_alloc_in_table(table, slot_index));
  ++slot_index;
  
  unsigned int max_word_index = ((slot_num-1) * COLOR_BITS_PER_OBJ) / BITS_PER_WORD;
  
  unsigned int color_bits_index = slot_index * COLOR_BITS_PER_OBJ;
  unsigned int word_index = color_bits_index / BITS_PER_WORD;
  unsigned int index_in_word = color_bits_index % BITS_PER_WORD;
  
  for(; word_index <= max_word_index; ++word_index, index_in_word = 0){
    if(table[word_index] == cur_alloc_mask)
      continue;
    index_in_word = next_free_index_in_color_word(table[word_index], index_in_word);
    if(index_in_word != MAX_SLOT_INDEX){
      assert(index_in_word < BITS_PER_WORD);
      return composed_slot_index(word_index, index_in_word);
    }
  }
  
  return MAX_SLOT_INDEX;
}

/* Only used in sspace compaction after sweeping now */
inline unsigned int next_alloc_slot_index_in_table(POINTER_SIZE_INT *table, unsigned int slot_index, unsigned int slot_num)
{
  unsigned int max_word_index = ((slot_num-1) * COLOR_BITS_PER_OBJ) / BITS_PER_WORD;
  
  unsigned int color_bits_index = slot_index * COLOR_BITS_PER_OBJ;
  unsigned int word_index = color_bits_index / BITS_PER_WORD;
  unsigned int index_in_word = color_bits_index % BITS_PER_WORD;
  
  for(; word_index <= max_word_index; ++word_index, index_in_word = 0){
    if(!table[word_index])
      continue;
    index_in_word = next_alloc_index_in_color_word(table[word_index], index_in_word);
    if(index_in_word != MAX_SLOT_INDEX){
      assert(index_in_word < BITS_PER_WORD);
      return composed_slot_index(word_index, index_in_word);
    }
  }
  
  return MAX_SLOT_INDEX;
}

inline Partial_Reveal_Object *next_alloc_slot_in_chunk(Chunk_Header *chunk)
{
  POINTER_SIZE_INT *table = chunk->table;
  
  unsigned int slot_index = next_alloc_slot_index_in_table(table, chunk->slot_index, chunk->slot_num);
  assert((slot_index == MAX_SLOT_INDEX)
            || (slot_index < chunk->slot_num) && slot_is_alloc_in_table(table, slot_index));
  if(slot_index == MAX_SLOT_INDEX)
    return NULL;
  Partial_Reveal_Object *p_obj = (Partial_Reveal_Object*)slot_index_to_addr(chunk, slot_index);
  chunk->slot_index = slot_index + 1;
  return p_obj;
}

inline void clear_free_slot_in_table(POINTER_SIZE_INT *table, unsigned int ceiling_slot_index)
{
  assert(ceiling_slot_index && ceiling_slot_index != MAX_SLOT_INDEX);
  unsigned int index_word_num = ceiling_slot_index / SLOT_NUM_PER_WORD_IN_TABLE;
  memset(table, 0, BYTES_PER_WORD*index_word_num);
  unsigned int bits_need_clear = ceiling_slot_index % SLOT_NUM_PER_WORD_IN_TABLE;
  if(!bits_need_clear) return;
  POINTER_SIZE_INT bit_mask = ~(((POINTER_SIZE_INT)1 << (bits_need_clear*COLOR_BITS_PER_OBJ)) - 1);
  table[index_word_num] &= bit_mask;
}

inline void alloc_slot_in_table(POINTER_SIZE_INT *table, unsigned int slot_index)
{
  assert(!slot_is_alloc_in_table(table, slot_index));
  
  unsigned int color_bits_index = slot_index * COLOR_BITS_PER_OBJ;
  unsigned int word_index = color_bits_index / BITS_PER_WORD;
  unsigned int index_in_word = color_bits_index % BITS_PER_WORD;
  
  table[word_index] |= cur_alloc_color << index_in_word;
}

/* We don't enable fresh chunk alloc for now,
 * because we observed perf down for the extra conditional statement when no many fresh chunks.
 */
//#define ENABLE_FRESH_CHUNK_ALLOC

/* 1. No need of synchronization. This is an allocator local chunk.
 * 2. If this chunk runs out of space, clear the chunk pointer.
 *    So it is important to give a parameter which is a local chunk pointer of a allocator while invoking this func.
 */
inline void *alloc_in_chunk(Chunk_Header* &chunk)
{
  POINTER_SIZE_INT *table = chunk->table;
  unsigned int slot_index = chunk->slot_index;
  
  assert(chunk->alloc_num < chunk->slot_num);
  ++chunk->alloc_num;
  assert(chunk->base);
  void *p_obj = (void*)((POINTER_SIZE_INT)chunk->base + ((POINTER_SIZE_INT)chunk->slot_size * slot_index));
#ifdef _DEBUG  
  slot_is_free_in_table(table, slot_index);
#endif
  alloc_slot_in_table(table, slot_index);
  if(chunk->status & CHUNK_NEED_ZEROING)
    memset(p_obj, 0, chunk->slot_size);
#ifdef SSPACE_VERIFY
  sspace_verify_free_area((POINTER_SIZE_INT*)p_obj, chunk->slot_size);
#endif

#ifdef ENABLE_FRESH_CHUNK_ALLOC
  if(chunk->status & CHUNK_FRESH){
    ++slot_index;
    chunk->slot_index = (slot_index < chunk->slot_num) ? slot_index : MAX_SLOT_INDEX;
  } else
#endif

  if(p_obj && gc_is_concurrent_mark_phase()) obj_mark_black_in_table((Partial_Reveal_Object*)p_obj,chunk->slot_size);

    chunk->slot_index = next_free_slot_index_in_table(table, slot_index, chunk->slot_num);
  if(chunk->slot_index == MAX_SLOT_INDEX){
    chunk->status = CHUNK_USED | CHUNK_NORMAL;
    chunk = NULL;
  }
  
  assert(!chunk || chunk->slot_index < chunk->slot_num);
  return p_obj;
}

inline void set_super_obj_mask(void *large_obj)
{ ((Partial_Reveal_Object*)large_obj)->obj_info |= SUPER_OBJ_MASK; }

#endif // _SSPACE_ALLOC_H_