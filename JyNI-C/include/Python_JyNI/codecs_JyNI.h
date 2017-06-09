/* This File is based on codecs.h from CPython 2.7.10 release.
 * It has been modified to suit JyNI needs.
 *
 *
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016, 2017 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
 * 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017
 * Python Software Foundation.
 *
 *
 * This file is part of JyNI.
 *
 * JyNI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * JyNI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with JyNI.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef Py_CODECREGISTRY_H
#define Py_CODECREGISTRY_H
#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------

   Python Codec Registry and support functions


Written by Marc-Andre Lemburg (mal@lemburg.com).

Copyright (c) Corporation for National Research Initiatives.

   ------------------------------------------------------------------------ */

/* Register a new codec search function.

   As side effect, this tries to load the encodings package, if not
   yet done, to make sure that it is always first in the list of
   search functions.

   The search_function's refcount is incremented by this function. */

PyAPI_FUNC(int) PyCodec_Register(
       PyObject *search_function
       );

/* Codec register lookup API.

   Looks up the given encoding and returns a CodecInfo object with
   function attributes which implement the different aspects of
   processing the encoding.

   The encoding string is looked up converted to all lower-case
   characters. This makes encodings looked up through this mechanism
   effectively case-insensitive.

   If no codec is found, a KeyError is set and NULL returned.

   As side effect, this tries to load the encodings package, if not
   yet done. This is part of the lazy load strategy for the encodings
   package.

 */

PyAPI_FUNC(PyObject *) _PyCodec_Lookup(
       const char *encoding
       );

/* Generic codec based encoding API.

   object is passed through the encoder function found for the given
   encoding using the error handling method defined by errors. errors
   may be NULL to use the default method defined for the codec.
   
   Raises a LookupError in case no encoder can be found.

 */

PyAPI_FUNC(PyObject *) PyCodec_Encode(
       PyObject *object,
       const char *encoding,
       const char *errors
       );

/* Generic codec based decoding API.

   object is passed through the decoder function found for the given
   encoding using the error handling method defined by errors. errors
   may be NULL to use the default method defined for the codec.
   
   Raises a LookupError in case no encoder can be found.

 */

PyAPI_FUNC(PyObject *) PyCodec_Decode(
       PyObject *object,
       const char *encoding,
       const char *errors
       );

/* Text codec specific encoding and decoding API.

   Checks the encoding against a list of codecs which do not
   implement a unicode<->bytes encoding before attempting the
   operation.

   Please note that these APIs are internal and should not
   be used in Python C extensions.

   XXX (ncoghlan): should we make these, or something like them, public
   in Python 3.5+?

 */
PyAPI_FUNC(PyObject *) _PyCodec_LookupTextEncoding(
       const char *encoding,
       const char *alternate_command
       );

PyAPI_FUNC(PyObject *) _PyCodec_EncodeText(
       PyObject *object,
       const char *encoding,
       const char *errors
       );

PyAPI_FUNC(PyObject *) _PyCodec_DecodeText(
       PyObject *object,
       const char *encoding,
       const char *errors
       );

/* These two aren't actually text encoding specific, but _io.TextIOWrapper
 * is the only current API consumer.
 */
PyAPI_FUNC(PyObject *) _PyCodecInfo_GetIncrementalDecoder(
       PyObject *codec_info,
       const char *errors
       );

PyAPI_FUNC(PyObject *) _PyCodecInfo_GetIncrementalEncoder(
       PyObject *codec_info,
       const char *errors
       );



/* --- Codec Lookup APIs -------------------------------------------------- 

   All APIs return a codec object with incremented refcount and are
   based on _PyCodec_Lookup().  The same comments w/r to the encoding
   name also apply to these APIs.

*/

/* Get an encoder function for the given encoding. */

PyAPI_FUNC(PyObject *) PyCodec_Encoder(
       const char *encoding
       );

/* Get a decoder function for the given encoding. */

PyAPI_FUNC(PyObject *) PyCodec_Decoder(
       const char *encoding
       );

/* Get a IncrementalEncoder object for the given encoding. */

PyAPI_FUNC(PyObject *) PyCodec_IncrementalEncoder(
       const char *encoding,
       const char *errors
       );

/* Get a IncrementalDecoder object function for the given encoding. */

PyAPI_FUNC(PyObject *) PyCodec_IncrementalDecoder(
       const char *encoding,
       const char *errors
       );

/* Get a StreamReader factory function for the given encoding. */

PyAPI_FUNC(PyObject *) PyCodec_StreamReader(
       const char *encoding,
       PyObject *stream,
       const char *errors
       );

/* Get a StreamWriter factory function for the given encoding. */

PyAPI_FUNC(PyObject *) PyCodec_StreamWriter(
       const char *encoding,
       PyObject *stream,
       const char *errors
       );

/* Unicode encoding error handling callback registry API */

/* Register the error handling callback function error under the given
   name. This function will be called by the codec when it encounters
   unencodable characters/undecodable bytes and doesn't know the
   callback name, when name is specified as the error parameter
   in the call to the encode/decode function.
   Return 0 on success, -1 on error */
PyAPI_FUNC(int) PyCodec_RegisterError(const char *name, PyObject *error);

/* Lookup the error handling callback function registered under the given
   name. As a special case NULL can be passed, in which case
   the error handling callback for "strict" will be returned. */
PyAPI_FUNC(PyObject *) PyCodec_LookupError(const char *name);

/* raise exc as an exception */
PyAPI_FUNC(PyObject *) PyCodec_StrictErrors(PyObject *exc);

/* ignore the unicode error, skipping the faulty input */
PyAPI_FUNC(PyObject *) PyCodec_IgnoreErrors(PyObject *exc);

/* replace the unicode encode error with ? or U+FFFD */
PyAPI_FUNC(PyObject *) PyCodec_ReplaceErrors(PyObject *exc);

/* replace the unicode encode error with XML character references */
PyAPI_FUNC(PyObject *) PyCodec_XMLCharRefReplaceErrors(PyObject *exc);

/* replace the unicode encode error with backslash escapes (\x, \u and \U) */
PyAPI_FUNC(PyObject *) PyCodec_BackslashReplaceErrors(PyObject *exc);

#ifdef __cplusplus
}
#endif
#endif /* !Py_CODECREGISTRY_H */
