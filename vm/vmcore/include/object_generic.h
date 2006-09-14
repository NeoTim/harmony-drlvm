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
 * @author Intel, Salikh Zakirov, Alexei Fedotov
 * @version $Revision: 1.1.2.1.4.4 $
 */  
#ifndef _OBJECT_GENERIC_H
#define _OBJECT_GENERIC_H

#include "object_layout.h"

#ifdef __cplusplus
extern "C" {
#endif

void
java_lang_Object_notifyAll(jobject);

jint
java_lang_Object_wait(jobject, jlong);

void
java_lang_Object_notify(jobject);

void
java_lang_Object_registerNatives(ManagedObject*);

long
generic_hashcode(ManagedObject*);

/* $$$ GMJ - collides w/ a defn elsewhere
int32
default_hashcode(ManagedObject*);
*/

#ifdef __cplusplus
}
#endif

#endif /* _OBJECT_GENERIC_H */
