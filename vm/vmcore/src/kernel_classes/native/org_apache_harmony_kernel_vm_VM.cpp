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
 * @file org_apache_harmony_kernel_vm_VM.cpp
 *
 * This file is a part of kernel class natives VM core component.
 * It contains implementation for native methods of org.apache.harmony.kernel.vm.VM kernel
 * class. Not all of the methods are implemented now.
 */

#include <assert.h>

#define LOG_DOMAIN "vm.accessors"
#include "jni_utils.h"

#include "vm_strings.h"
#include "org_apache_harmony_kernel_vm_VM.h"
#include "java_lang_VMClassRegistry.h"

/**
 * Implements java.lang.String.intern(..) method.
 * For details see kernel classes component documentation.
 */
JNIEXPORT jobject JNICALL Java_org_apache_harmony_kernel_vm_VM_getClassLoader
(JNIEnv *jenv, jclass, jclass clazz)
{
    // reuse similar method in VMClassRegistry
    return Java_java_lang_VMClassRegistry_getClassLoader(jenv, NULL, clazz);
}

/**
 * Implements org.apache.harmony.vm.VM.intern0(..) method.
 * For details see kernel classes component documentation.
 */
JNIEXPORT jstring JNICALL
Java_org_apache_harmony_kernel_vm_VM_intern0(JNIEnv *jenv, jclass, jstring str)
{
    // call corresponding VM internal function
    return string_intern(jenv, str);
}
