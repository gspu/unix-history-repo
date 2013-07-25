/**
 * @copyright
 * ====================================================================
 *    Licensed to the Apache Software Foundation (ASF) under one
 *    or more contributor license agreements.  See the NOTICE file
 *    distributed with this work for additional information
 *    regarding copyright ownership.  The ASF licenses this file
 *    to you under the Apache License, Version 2.0 (the
 *    "License"); you may not use this file except in compliance
 *    with the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing,
 *    software distributed under the License is distributed on an
 *    "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *    KIND, either express or implied.  See the License for the
 *    specific language governing permissions and limitations
 *    under the License.
 * ====================================================================
 * @endcopyright
 *
 * @file svn_delta_private.h
 * @brief The Subversion delta/diff/editor library - Internal routines
 */

#ifndef SVN_DELTA_PRIVATE_H
#define SVN_DELTA_PRIVATE_H

#include <apr_pools.h>

#include "svn_types.h"
#include "svn_error.h"
#include "svn_delta.h"
#include "svn_editor.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef svn_error_t *(*svn_delta__start_edit_func_t)(
  void *baton,
  svn_revnum_t base_revision);

typedef svn_error_t *(*svn_delta__target_revision_func_t)(
  void *baton,
  svn_revnum_t target_revision,
  apr_pool_t *scratch_pool);

typedef svn_error_t *(*svn_delta__unlock_func_t)(
  void *baton,
  const char *path,
  apr_pool_t *scratch_pool);


/* See svn_editor__insert_shims() for more information. */
struct svn_delta__extra_baton
{
  svn_delta__start_edit_func_t start_edit;
  svn_delta__target_revision_func_t target_revision;
  void *baton;
};


/** A temporary API to convert from a delta editor to an Ev2 editor. */
svn_error_t *
svn_delta__editor_from_delta(svn_editor_t **editor_p,
                             struct svn_delta__extra_baton **exb,
                             svn_delta__unlock_func_t *unlock_func,
                             void **unlock_baton,
                             const svn_delta_editor_t *deditor,
                             void *dedit_baton,
                             svn_boolean_t *send_abs_paths,
                             const char *repos_root,
                             const char *base_relpath,
                             svn_cancel_func_t cancel_func,
                             void *cancel_baton,
                             svn_delta_fetch_kind_func_t fetch_kind_func,
                             void *fetch_kind_baton,
                             svn_delta_fetch_props_func_t fetch_props_func,
                             void *fetch_props_baton,
                             apr_pool_t *result_pool,
                             apr_pool_t *scratch_pool);


/** A temporary API to convert from an Ev2 editor to a delta editor. */
svn_error_t *
svn_delta__delta_from_editor(const svn_delta_editor_t **deditor,
                             void **dedit_baton,
                             svn_editor_t *editor,
                             svn_delta__unlock_func_t unlock_func,
                             void *unlock_baton,
                             svn_boolean_t *found_abs_paths,
                             const char *repos_root,
                             const char *base_relpath,
                             svn_delta_fetch_props_func_t fetch_props_func,
                             void *fetch_props_baton,
                             svn_delta_fetch_base_func_t fetch_base_func,
                             void *fetch_base_baton,
                             struct svn_delta__extra_baton *exb,
                             apr_pool_t *pool);

/**
 * Get the data from IN, compress it according to the specified
 * COMPRESSION_LEVEL and write the result to OUT.
 * SVN_DELTA_COMPRESSION_LEVEL_NONE is valid for COMPRESSION_LEVEL.
 */
svn_error_t *
svn__compress(svn_string_t *in,
              svn_stringbuf_t *out,
              int compression_level);

/**
 * Get the compressed data from IN, decompress it and write the result to
 * OUT.  Return an error if the decompressed size is larger than LIMIT.
 */
svn_error_t *
svn__decompress(svn_string_t *in,
                svn_stringbuf_t *out,
                apr_size_t limit);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SVN_DELTA_PRIVATE_H */
