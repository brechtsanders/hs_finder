/*
 * Copyright (c) 2018, Brecht Sanders
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

 /**
 * @file      hs_finder.h
 * @brief     hs_finder header file with main functions
 * @author    Brecht Sanders
 * @date      2018
 * @copyright BSD
 *
 * This header file defines the functions needed search multiple patterns in a stream of (text) data.
 * It allows transforming the input stream into a modified output stream.
 * There is also a possibility to search in multiple passes, which is useful when expressions overlap with eachother.
 * This library depends on Hyperscan (https://www.hyperscan.io/) and is also released under a 3-clause BSD license.
 */

#ifndef INCLUDED_HS_FINDER_H
#define INCLUDED_HS_FINDER_H

#include <stdlib.h>
#include <stdio.h>
#include <hs/hs.h>

/*! \cond PRIVATE */
#ifdef _WIN32
#if defined(BUILD_HS_FINDER_DLL)
#define DLL_EXPORT_HS_FINDER __declspec(dllexport)
#elif !defined(STATIC) && !defined(BUILD_HS_FINDER_STATIC)
#define DLL_EXPORT_HS_FINDER __declspec(dllimport)
#else
#define DLL_EXPORT_HS_FINDER
#endif
#else
#define DLL_EXPORT_HS_FINDER
#endif
/*! \endcond */

/*! \brief version number constants
 * \sa     hs_finder_get_version()
 * \sa     hs_finder_get_version_string()
 * \name   HS_FINDER_VERSION_*
 * \{
 */
/*! \brief major version number */
#define HS_FINDER_VERSION_MAJOR 0
/*! \brief minor version number */
#define HS_FINDER_VERSION_MINOR 1
/*! \brief micro version number */
#define HS_FINDER_VERSION_MICRO 0
/*! @} */

/*! \cond PRIVATE */
#define HS_FINDER_VERSION_STRINGIZE_(major, minor, micro) #major"."#minor"."#micro
#define HS_FINDER_VERSION_STRINGIZE(major, minor, micro) HS_FINDER_VERSION_STRINGIZE_(major, minor, micro)
/*! \endcond */

/*! \brief string with dotted version number \hideinitializer */
#define HS_FINDER_VERSION_STRING HS_FINDER_VERSION_STRINGIZE(HS_FINDER_VERSION_MAJOR, HS_FINDER_VERSION_MINOR, HS_FINDER_VERSION_MICRO)

/*! \brief string with name of library */
#define HS_FINDER_NAME "hs_finder"

/*! \brief string with name and version of library \hideinitializer */
#define HS_FINDER_FULLNAME HS_FINDER_NAME " " HS_FINDER_VERSION_STRING

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief get hs_finder version string
 * \param  pmajor        pointer to integer that will receive major version number
 * \param  pminor        pointer to integer that will receive minor version number
 * \param  pmicro        pointer to integer that will receive micro version number
 * \sa     hs_finder_get_version_string()
 */
DLL_EXPORT_HS_FINDER void hs_finder_get_version (int* pmajor, int* pminor, int* pmicro);

/*! \brief get hs_finder version string
 * \return version string
 * \sa     hs_finder_get_version()
 */
DLL_EXPORT_HS_FINDER const char* hs_finder_get_version_string ();

/*! \brief hs_finder object type */
struct hs_finder;

/*! \brief type of pointer to function for processing matches (basically the same as hyperscan's match_event_handler)
 * \param  id              match id as specified in hs_finder_add_expr()
 * \param  from            start position of match (requires HS_FLAG_SOM_LEFTMOST flag in hs_finder_add_expr())
 * \param  to              end position of match
 * \param  flags           flags used for match (as specified in in hs_finder_add_expr())
 * \param  finder          hs_finder object
 * \return zero to continue, or non-zero to abort further matching
 * \sa     hs_finder_add_expr()
 * \sa     hs_finder_open()
 * \sa     hs_finder_process()
 */
typedef int (*hs_finder_match_fn)(unsigned int id, unsigned long long from, unsigned long long to, unsigned int flags, struct hs_finder* finder);

/*! \brief type of pointer to function for processing output
 * \param  callbackdata    custom data as passed to hs_finder_open()
 * \param  data            data to be processed
 * \param  datalen         length of data to be processed
 * \return (unused)
 * \sa     hs_finder_open()
 * \sa     hs_finder_output_to_stream()
 */
typedef size_t (*hs_finder_output_fn) (void* callbackdata, const char* data, size_t datalen);

/*! \brief initialize hs_finder object
 * \param  matchfn         function to call for each match
 * \return allocated hs_finder object (or NULL on error)
 * \sa     hs_finder_output_fn
 * \sa     hs_finder_cleanup()
 */
DLL_EXPORT_HS_FINDER struct hs_finder* hs_finder_initialize (hs_finder_match_fn matchfn, void* callbackdata);

/*! \brief clean up hs_finder object
 * \param  finder          hs_finder object
 * \sa     hs_finder_initialize()
 */
DLL_EXPORT_HS_FINDER void hs_finder_cleanup (struct hs_finder* finder);

/*! \brief add search expression to hs_finder object
 * \param  finder          hs_finder object
 * \param  expr            matching expression
 * \param  flags           matching flags (HS_FLAG_*, make sure to include HS_FLAG_SOM_LEFTMOST to use the from parameter in the match function)
 * \param  id              matching id
 * \sa     hs_finder_initialize()
 * \sa     hs_finder_process()
 */
DLL_EXPORT_HS_FINDER void hs_finder_add_expr (struct hs_finder* finder, char* expr, unsigned int flags, unsigned int id);

/*! \brief add search instance to hs_finder object
 * \param  finder          hs_finder object
 * \param  matchfn         function to call for each match
 * \sa     hs_finder_initialize()
 * \sa     hs_finder_process()
 */
DLL_EXPORT_HS_FINDER void hs_finder_add_instance (struct hs_finder* finder, hs_finder_match_fn matchfn, void* callbackdata);

/*! \brief function (of type hs_finder_output_fn) to write data to a FILE* stream
 * \param  callbackdata    output stream (of type FILE*)
 * \param  data            data to be written
 * \param  datalen         length of data to be written
 * \return number of bytes written
 * \sa     hs_finder_output_fn
 */
DLL_EXPORT_HS_FINDER size_t hs_finder_output_to_stream (void* callbackdata, const char* data, size_t datalen);

/*! \brief function (of type hs_finder_output_fn) to discard data
 * \param  callbackdata    not used
 * \param  data            data to be written
 * \param  datalen         length of data to be written
 * \return number of bytes written
 * \sa     hs_finder_output_fn
 */
DLL_EXPORT_HS_FINDER size_t hs_finder_output_to_null (void* callbackdata, const char* data, size_t datalen);

/*! \brief open data stream for searching
 * \param  finder          hs_finder object
 * \param  outputfn        function to call for processing output (if NULL will use hs_finder_output_to_stream)
 * \param  callbackdata    custom data to be passed to \p outputfn
 * \return HS_SUCCESS on success
 * \sa     hs_finder_process()
 * \sa     hs_finder_close()
 * \sa     hs_finder_output_fn
 */
DLL_EXPORT_HS_FINDER hs_error_t hs_finder_open (struct hs_finder* finder, hs_finder_output_fn outputfn, void* callbackdata);

/*! \brief process chunk of data for searching
 * \param  finder          hs_finder object
 * \param  data            data to be processed
 * \param  datalen         length of data to be processed
 * \return HS_SUCCESS on success
 * \sa     hs_finder_open()
 * \sa     hs_finder_close()
 * \sa     hs_finder_add_expr()
 * \sa     hs_finder_add_instance()
 */
DLL_EXPORT_HS_FINDER hs_error_t hs_finder_process (struct hs_finder* finder, const char* data, size_t datalen);

/*! \brief close data stream
 * \param  finder          hs_finder object
 * \return HS_SUCCESS on success
 * \sa     hs_finder_open()
 * \sa     hs_finder_process()
 */
DLL_EXPORT_HS_FINDER hs_error_t hs_finder_close (struct hs_finder* finder);

/*! \brief get position in input data stream
 * \param  finder          hs_finder object
 * \return current position in input data stream
 * \sa     hs_finder_open()
 * \sa     hs_finder_process()
 * \sa     hs_finder_flush()
 * \sa     hs_finder_skip()
 */
DLL_EXPORT_HS_FINDER size_t hs_finder_get_pos (struct hs_finder* finder);

/*! \brief get callback data, to be used inside hs_finder_match_fn
 * \param  finder          hs_finder object
 * \return callback data
 * \sa     hs_finder_match_fn
 * \sa     hs_finder_initialize()
 * \sa     hs_finder_add_instance()
 * \sa     hs_finder_open()
 * \sa     hs_finder_process()
 */
DLL_EXPORT_HS_FINDER void* hs_finder_get_callbackdata (struct hs_finder* finder);

/*! \brief flush data from input stream to output, to be used inside hs_finder_match_fn
 * \param  finder          hs_finder object
 * \param  flushpos        input position to flush data up to
 * \return current position in input data stream (after flushing)
 * \sa     hs_finder_match_fn
 * \sa     hs_finder_open()
 * \sa     hs_finder_process()
 * \sa     hs_finder_get_pos()
 * \sa     hs_finder_skip()
 * \sa     hs_finder_output()
 * \sa     hs_finder_output_fn
 */
DLL_EXPORT_HS_FINDER size_t hs_finder_flush (struct hs_finder* finder, size_t flushpos);

/*! \brief discard data from input stream, to be used inside hs_finder_match_fn
 * \param  finder          hs_finder object
 * \param  flushpos        input position to discard data up to
 * \return current position in input data stream (after discarding)
 * \sa     hs_finder_match_fn
 * \sa     hs_finder_open()
 * \sa     hs_finder_process()
 * \sa     hs_finder_get_pos()
 * \sa     hs_finder_flush()
 */
DLL_EXPORT_HS_FINDER size_t hs_finder_skip (struct hs_finder* finder, size_t flushpos);

/*! \brief output chunk of data to output (make sure to call hs_finder_flush() before), to be used inside hs_finder_match_fn
 * \param  finder          hs_finder object
 * \param  data            data to be sent
 * \param  datalen         length of data to be sent
 * \return value returned by hs_finder_output_fn
 * \sa     hs_finder_match_fn
 * \sa     hs_finder_open()
 * \sa     hs_finder_process()
 * \sa     hs_finder_flush()
 * \sa     hs_finder_output_fn
 */
DLL_EXPORT_HS_FINDER size_t hs_finder_output (struct hs_finder* finder, const char* data, size_t datalen);

#ifdef __cplusplus
}
#endif

#endif //INCLUDED_HS_FINDER_H
