/*
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013 Python Software Foundation.  All rights reserved.
 *
 * Copyright of JyNI:
 * Copyright (c) 2013 Stefan Richthofer.  All rights reserved.
 *
 *
 * This file is part of JyNI.
 *
 * JyNI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JyNI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JyNI.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Linking this library statically or dynamically with other modules is
 * making a combined work based on this library.  Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * As a special exception, the copyright holders of this library give you
 * permission to link this library with independent modules to produce an
 * executable, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting executable under
 * terms of your choice, provided that you also meet, for each linked
 * independent module, the terms and conditions of the license of that
 * module.  An independent module is a module which is not derived from
 * or based on this library.  If you modify this library, you may extend
 * this exception to your version of the library, but you are not
 * obligated to do so.  If you do not wish to do so, delete this
 * exception statement from your version.
 */


/*
 * JyNILoader.c
 *
 *  Created on: 13.04.2013, 07:55:05
 *      Author: Stefan Richthofer
 *
 * The sole purpose of this loader is to enable JyNI to use the
 * RTLD_GLOBAL flag properly on loading extensions dynamically.
 * JNI does not use this flag and we can't change that behaviour
 * directly. So we need this intermediate step to load JyNI with
 * RTLD_GLOBAL set.
 */

#include <JyNI_JyNI.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <stdio.h>
#include <string.h>

#if defined(__NetBSD__)
#include <sys/param.h>
#if (NetBSD < 199712)
#include <nlist.h>
#include <link.h>
#define dlerror() "error in dynamic linking"
#endif
#endif /* NetBSD */

#include <dlfcn.h>

JavaVM* java;
void* JyNIHandle;

jobject (*JyNILoadModule)(JNIEnv*, jclass, jstring, jstring);
void (*JyNIClearPyCPeer)(JNIEnv*, jclass, jlong, jlong);
jobject (*JyNICallPyCPeer)(JNIEnv*, jclass, jlong, jobject, jobject);
jobject (*JyNIGetAttrString)(JNIEnv*, jclass, jlong, jstring);
jobject (*JyNIrepr)(JNIEnv*, jclass, jlong);
jstring (*JyNIPyObjectAsString)(JNIEnv*, jclass, jlong);
jobject (*JyNIPyObjectAsPyString)(JNIEnv*, jclass, jlong);
jint (*JyNISetAttrString)(JNIEnv*, jclass, jlong, jstring, jobject);
void (*JyNIUnload)(JavaVM*);

jobject (*JyList_get)(JNIEnv*, jclass, jlong, jint);
jint (*JyList_size)(JNIEnv*, jclass, jlong);
jobject (*JyList_set)(JNIEnv*, jclass, jlong, jint, jobject, jlong);
void (*JyList_add)(JNIEnv*, jclass, jlong, jint, jobject, jlong);
jobject (*JyList_remove)(JNIEnv*, jclass, jlong, jint);

//void JyNI_unload(JavaVM *jvm);


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
	java = jvm; // cache the JavaVM pointer
	return JNI_VERSION_1_2;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved)
{
	(*JyNIUnload)(jvm);
}

/*
 * Class:     JyNI_JyNI
 * Method:    initJyNI
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_initJyNI
  (JNIEnv * env, jclass class, jstring JyNILibPath)
{
	//puts("initJyNI...");
	//puts("path:");
	jboolean isCopy;

	char* utf_string = (*env)->GetStringUTFChars(env, JyNILibPath, &isCopy);
	//"+1" for 0-termination:
	char mPath[strlen(utf_string)+1];
	strcpy(mPath, utf_string);
	(*env)->ReleaseStringUTFChars(env, JyNILibPath, utf_string);
	//puts(mPath);

	//JyNIHandle = dlopen(mPath, JyNI_JyNI_RTLD_LAZY | JyNI_JyNI_RTLD_GLOBAL);
	JyNIHandle = dlopen(mPath, RTLD_LAZY | RTLD_GLOBAL);
	//printf("%i\n", (int) (RTLD_LAZY | RTLD_GLOBAL));
	//printf("%i\n", (int) JyNI_JyNI_RTLD_LAZY);

	if (JyNIHandle == NULL) {
		const char *error = dlerror();
		if (error == NULL)
			error = "unknown dlopen() error in JyNILoader";
		puts("dlopen-error in JyNILoader:");
		puts(error);
		//PyErr_SetString(PyExc_ImportError, error);
		//JyNI_JyErr_SetString((*env)->GetStaticObjectField(env, pyPyClass, pyPyImportError), error);
		//return NULL;
	}
	//jint JyNI_init(JavaVM *jvm);
	jint (*JyNIInit)(JavaVM*);
	//puts("call dlsym...");
	*(void **) (&JyNIInit) = dlsym(JyNIHandle, "JyNI_init");
	*(void **) (&JyNILoadModule) = dlsym(JyNIHandle, "JyNI_loadModule");
	*(void **) (&JyNIClearPyCPeer) = dlsym(JyNIHandle, "JyNI_clearPyCPeer");
	*(void **) (&JyNICallPyCPeer) = dlsym(JyNIHandle, "JyNI_callPyCPeer");
	*(void **) (&JyNIGetAttrString) = dlsym(JyNIHandle, "JyNI_getAttrString");
	*(void **) (&JyNIrepr) = dlsym(JyNIHandle, "JyNI_repr");
	*(void **) (&JyNIPyObjectAsString) = dlsym(JyNIHandle, "JyNI_PyObjectAsString");
	*(void **) (&JyNIPyObjectAsPyString) = dlsym(JyNIHandle, "JyNI_PyObjectAsPyString");
	*(void **) (&JyNISetAttrString) = dlsym(JyNIHandle, "JyNI_setAttrString");
	*(void **) (&JyNIUnload) = dlsym(JyNIHandle, "JyNI_unload");

	*(void **) (&JyList_get) = dlsym(JyNIHandle, "JyList_get");
	*(void **) (&JyList_size) = dlsym(JyNIHandle, "JyList_size");
	*(void **) (&JyList_set) = dlsym(JyNIHandle, "JyList_set");
	*(void **) (&JyList_add) = dlsym(JyNIHandle, "JyList_add");
	*(void **) (&JyList_remove) = dlsym(JyNIHandle, "JyList_remove");

	//puts("done");
	jint result = (*JyNIInit)(java);
	if (result != JNI_VERSION_1_2) puts("Init-result indicates error!");
	//else puts("Init-result indicates success!");

}

/*
 * Class:     JyNI_JyNI
 * Method:    loadModule
 * Signature: (Ljava/lang/String;Ljava/lang/String;)LJyNI/JyNIModule;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_loadModule
  (JNIEnv *env, jclass class, jstring moduleName, jstring modulePath)
{
	return (*JyNILoadModule)(env, class, moduleName, modulePath);
}

/*
 * Class:     JyNI_JyNI
 * Method:    clearPyCPeer
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_clearPyCPeer
  (JNIEnv *env, jclass class, jlong objectHandle, jlong refHandle)
{
	(*JyNIClearPyCPeer)(env, class, objectHandle, refHandle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    callPyCPeer
 * Signature: (JLorg/python/core/PyObject;Lorg/python/core/PyObject;)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_callPyCPeer
  (JNIEnv *env, jclass class, jlong peerHandle, jobject args, jobject kw)
{
	return (*JyNICallPyCPeer)(env, class, peerHandle, args, kw);
}

/*
 * Class:     JyNI_JyNI
 * Method:    getAttrString
 * Signature: (JLjava/lang/String;)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_getAttrString(JNIEnv *env, jclass class, jlong handle, jstring name)
{
	return (*JyNIGetAttrString)(env, class, handle, name);
}

/*
 * Class:     JyNI_JyNI
 * Method:    setAttrString
 * Signature: (JLjava/lang/String;Lorg/python/core/PyObject;)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_setAttrString(JNIEnv *env, jclass class, jlong handle, jstring name, jobject value)
{
	return (*JyNISetAttrString)(env, class, handle, name, value);
}

/*
 * Class:     JyNI_JyNI
 * Method:    repr
 * Signature: (J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_repr(JNIEnv *env, jclass class, jlong handle)
{
	return (*JyNIrepr)(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyObjectAsString
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_JyNI_JyNI_PyObjectAsString(JNIEnv *env, jclass class, jlong handle)
{
	return (*JyNIPyObjectAsString)(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    PyObjectAsPyString
 * Signature: (J)Lorg/python/core/PyString;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_PyObjectAsPyString(JNIEnv *env, jclass class, jlong handle)
{
	return (*JyNIPyObjectAsPyString)(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_get
 * Signature: (JI)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyList_1get(JNIEnv *env, jclass class, jlong handle, jint index)
{
	return (*JyList_get)(env, class, handle, index);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_size
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_JyNI_JyNI_JyList_1size(JNIEnv *env, jclass class, jlong handle)
{
	return (*JyList_size)(env, class, handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_set
 * Signature: (JILorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyList_1set(JNIEnv *env, jclass class, jlong handle, jint index, jobject obj, jlong pyObj)
{
	return (*JyList_set)(env, class, handle, index, obj, pyObj);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_add
 * Signature: (JILorg/python/core/PyObject;J)V
 */
JNIEXPORT void JNICALL Java_JyNI_JyNI_JyList_1add(JNIEnv *env, jclass class, jlong handle, jint index, jobject obj, jlong pyObj)
{
	(*JyList_add)(env, class, handle, index, obj, pyObj);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_remove
 * Signature: (JI)Lorg/python/core/PyObject;
 */
JNIEXPORT jobject JNICALL Java_JyNI_JyNI_JyList_1remove(JNIEnv *env, jclass class, jlong handle, jint index)
{
	return (*JyList_remove)(env, class, handle, index);
}