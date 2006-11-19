/*
 *  Copyright 2005-2006 The Apache Software Foundation or its licensors, as applicable.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @author Xiao-Feng Li, 2006/10/25
 */
#ifndef _GC_METADATA_H_
#define _GC_METADATA_H_

#include "gc_common.h"
#include "../utils/vector_block.h"
#include "../utils/sync_pool.h"

typedef struct GC_Metadata{  
  void* heap_start;
  void* heap_end;
  
  Pool* free_task_pool; /* list of free buffers for mark tasks */
  Pool* mark_task_pool; /* list of mark tasks */
  
  /* FIXME:: the mutator remset pool can be merged with the rootset pool*/
  Pool* free_set_pool; /* list of free buffers for rootsets remsets */
  Pool* gc_rootset_pool; /* list of root sets for enumeration */
  Pool* mutator_remset_pool; /* list of remsets generated by app during execution */
  Pool* collector_remset_pool; /* list of remsets generated by gc during collection */
  Pool* collector_repset_pool; /* list of repointed ref slot sets */
      
}GC_Metadata;

void gc_metadata_initialize(GC* gc);
void gc_metadata_destruct(GC* gc);
void gc_metadata_reset(GC* gc);

void gc_reset_rootset(GC* gc);
void gc_update_repointed_refs(Collector* collector);

void collector_marktask_add_entry(Collector* collector, Partial_Reveal_Object* p_obj);

void mutator_remset_add_entry(Mutator* mutator, Partial_Reveal_Object** p_slot);
void collector_remset_add_entry(Collector* collector, Partial_Reveal_Object** p_slot);
void gc_rootset_add_entry(GC* gc, Partial_Reveal_Object** p_slot);

void collector_repset_add_entry(Collector* collector, Partial_Reveal_Object** p_slot);

#endif /* #ifndef _GC_METADATA_H_ */
