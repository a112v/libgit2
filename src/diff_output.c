#include "git2/oid.h"
/*
 * A diff_delta_context represents all of the information that goes into
 * processing the diff of an observed file change.  In the case of the
 * git_diff_foreach() call it is an emphemeral structure that is filled
 * in to execute each diff.  In the case of a git_diff_iterator, it holds
 * most of the information for the diff in progress.
 *
 * As each delta is processed, it goes through 3 phases: prep, load, exec.
 *
 * - In the prep phase, we just set the delta and quickly check the file
 *   attributes to see if it should be treated as binary.
 * - In the load phase, we actually load the file content into memory.
 *   At this point, if we had deferred calculating OIDs, we might have to
 *   correct the delta to be UNMODIFIED.
 * - In the exec phase, we actually run the diff and execute the callbacks.
 *   For foreach, this is just a pass-through to the user's callbacks.  For
 *   iterators, we record the hunks and data spans into memory.
  */
	git_repository   *repo;
	git_diff_options *opts;
	xdemitconf_t xdiff_config;
	xpparam_t    xdiff_params;
	uint32_t prepped  : 1;
	uint32_t loaded   : 1;
	uint32_t diffable : 1;
	uint32_t diffed   : 1;
	git_iterator_type_t old_src;
	git_iterator_type_t new_src;
	git_blob *old_blob;
	git_blob *new_blob;
	git_map   old_data;
	git_map   new_data;
	void *cb_data;
	git_diff_hunk_fn per_hunk;
	git_diff_data_fn per_line;
	int cb_error;
} diff_delta_context;
static int parse_hunk_header(git_diff_range *range, const char *header)
	/* expect something of the form "@@ -%d[,%d] +%d[,%d] @@" */
	if (*header != '@')
		return -1;
	if (read_next_int(&header, &range->old_start) < 0)
		return -1;
	if (*header == ',') {
		if (read_next_int(&header, &range->old_lines) < 0)
	} else
		range->old_lines = 1;
	if (read_next_int(&header, &range->new_start) < 0)
		return -1;
	if (*header == ',') {
		if (read_next_int(&header, &range->new_lines) < 0)
	} else
		range->new_lines = 1;
	if (range->old_start < 0 || range->new_start < 0)
		return -1;
	return 0;
}
static int format_hunk_header(char *header, size_t len, git_diff_range *range)
{
	if (range->old_lines != 1) {
		if (range->new_lines != 1)
			return p_snprintf(
				header, len, "@@ -%d,%d +%d,%d @@",
				range->old_start, range->old_lines,
				range->new_start, range->new_lines);
		else
			return p_snprintf(
				header, len, "@@ -%d,%d +%d @@",
				range->old_start, range->old_lines, range->new_start);
	} else {
		if (range->new_lines != 1)
			return p_snprintf(
				header, len, "@@ -%d +%d,%d @@",
				range->old_start, range->new_start, range->new_lines);
		else
			return p_snprintf(
				header, len, "@@ -%d +%d @@",
				range->old_start, range->new_start);
}
static bool diff_delta_is_ambiguous(git_diff_delta *delta)
{
	return (git_oid_iszero(&delta->new_file.oid) &&
			(delta->new_file.flags & GIT_DIFF_FILE_VALID_OID) == 0 &&
			delta->status == GIT_DELTA_MODIFIED);
}
static bool diff_delta_should_skip(git_diff_options *opts, git_diff_delta *delta)
{
	if (delta->status == GIT_DELTA_UNMODIFIED &&
		(opts->flags & GIT_DIFF_INCLUDE_UNMODIFIED) == 0)
		return true;
	if (delta->status == GIT_DELTA_IGNORED &&
		(opts->flags & GIT_DIFF_INCLUDE_IGNORED) == 0)
		return true;
	if (delta->status == GIT_DELTA_UNTRACKED &&
		(opts->flags & GIT_DIFF_INCLUDE_UNTRACKED) == 0)
		return true;
	return false;
static int update_file_is_binary_by_attr(
	git_repository *repo, git_diff_file *file)

	/* because of blob diffs, cannot assume path is set */
	if (!file->path || !strlen(file->path))
		return 0;


#define NOT_BINARY_FLAGS (GIT_DIFF_FILE_NOT_BINARY|GIT_DIFF_FILE_NO_DATA)

	else if ((delta->old_file.flags & NOT_BINARY_FLAGS) != 0 &&
			 (delta->new_file.flags & NOT_BINARY_FLAGS) != 0)

static int diff_delta_is_binary_by_attr(diff_delta_context *ctxt)
	git_diff_delta *delta = ctxt->delta;
	if (ctxt->opts->flags & GIT_DIFF_FORCE_TEXT) {
	if (update_file_is_binary_by_attr(ctxt->repo, &delta->old_file) < 0)
		delta->new_file.flags |= (delta->old_file.flags & BINARY_DIFF_FLAGS);
		error = update_file_is_binary_by_attr(ctxt->repo, &delta->new_file);
static int diff_delta_is_binary_by_content(
	diff_delta_context *ctxt, git_diff_file *file, git_map *map)
	if ((file->flags & BINARY_DIFF_FLAGS) == 0) {
		search.ptr  = map->data;
		search.size = min(map->len, 4000);
			file->flags |= GIT_DIFF_FILE_BINARY;
			file->flags |= GIT_DIFF_FILE_NOT_BINARY;
	update_delta_is_binary(ctxt->delta);
	return 0;
}

static int diff_delta_is_binary_by_size(
	diff_delta_context *ctxt, git_diff_file *file)
{
	git_off_t threshold = MAX_DIFF_FILESIZE;

	if ((file->flags & BINARY_DIFF_FLAGS) != 0)
		return 0;

	if (ctxt && ctxt->opts) {
		if (ctxt->opts->max_size < 0)
			return 0;

		if (ctxt->opts->max_size > 0)
			threshold = ctxt->opts->max_size;
	if (file->size > threshold)
		file->flags |= GIT_DIFF_FILE_BINARY;
	update_delta_is_binary(ctxt->delta);
	diff_delta_context *ctxt,
	git_diff_file *file,
	int error;
	git_odb_object *odb_obj = NULL;

	if (git_oid_iszero(&file->oid))
	if (!file->size) {
		git_odb *odb;
		size_t len;
		git_otype type;

		/* peek at object header to avoid loading if too large */
		if ((error = git_repository_odb__weakptr(&odb, ctxt->repo)) < 0 ||
			(error = git_odb__read_header_or_object(
				&odb_obj, &len, &type, odb, &file->oid)) < 0)
			return error;

		assert(type == GIT_OBJ_BLOB);

		file->size = len;
	}

	/* if blob is too large to diff, mark as binary */
	if ((error = diff_delta_is_binary_by_size(ctxt, file)) < 0)
		return error;
	if (ctxt->delta->binary == 1)
		return 0;

	if (odb_obj != NULL) {
		error = git_object__from_odb_object(
			(git_object **)blob, ctxt->repo, odb_obj, GIT_OBJ_BLOB);
		git_odb_object_free(odb_obj);
	} else
		error = git_blob_lookup(blob, ctxt->repo, &file->oid);

	if (error)
		return error;

	return diff_delta_is_binary_by_content(ctxt, file, map);
	diff_delta_context *ctxt,
	const char *wd = git_repository_workdir(ctxt->repo);
	if (git_buf_joinpath(&path, wd, file->path) < 0)
		ssize_t alloc_len, read_len;
		/* link path on disk could be UTF-16, so prepare a buffer that is
		 * big enough to handle some UTF-8 data expansion
		 */
		alloc_len = (ssize_t)(file->size * 2) + 1;

		map->data = git__malloc(alloc_len);
		read_len = p_readlink(path.ptr, map->data, (int)alloc_len);
		if (read_len < 0) {
			goto cleanup;
		}

		map->len = read_len;
		git_file fd = git_futils_open_ro(path.ptr);
		git_vector filters = GIT_VECTOR_INIT;

		if (fd < 0) {
			error = fd;
			goto cleanup;
		}

		if (!file->size)
			file->size = git_futils_filesize(fd);

		if ((error = diff_delta_is_binary_by_size(ctxt, file)) < 0 ||
			ctxt->delta->binary == 1)
			goto close_and_cleanup;

		error = git_filters_load(
			&filters, ctxt->repo, file->path, GIT_FILTER_TO_ODB);
		if (error < 0)
			goto close_and_cleanup;

		if (error == 0) { /* note: git_filters_load returns filter count */
			error = git_futils_mmap_ro(map, fd, 0, (size_t)file->size);
			file->flags |= GIT_DIFF_FILE_UNMAP_DATA;
		} else {
			git_buf raw = GIT_BUF_INIT, filtered = GIT_BUF_INIT;

			if (!(error = git_futils_readbuffer_fd(&raw, fd, (size_t)file->size)) &&
				!(error = git_filters_apply(&filtered, &raw, &filters)))
			{
				map->len  = git_buf_len(&filtered);
				map->data = git_buf_detach(&filtered);

				file->flags |= GIT_DIFF_FILE_FREE_DATA;
			}

			git_buf_free(&raw);
			git_buf_free(&filtered);
		}

close_and_cleanup:
		git_filters_free(&filters);
		p_close(fd);
	}

	/* once data is loaded, update OID if we didn't have it previously */
	if (!error && (file->flags & GIT_DIFF_FILE_VALID_OID) == 0) {
		error = git_odb_hash(
			&file->oid, map->data, map->len, GIT_OBJ_BLOB);
		if (!error)
			file->flags |= GIT_DIFF_FILE_VALID_OID;

	if (!error)
		error = diff_delta_is_binary_by_content(ctxt, file, map);

cleanup:
		map->data = "";
		map->len  = 0;
		map->data = "";
		map->len  = 0;
static void diff_delta_init_context(
	diff_delta_context *ctxt,
	git_repository   *repo,
	git_diff_options *opts,
	git_iterator_type_t old_src,
	git_iterator_type_t new_src)
{
	memset(ctxt, 0, sizeof(diff_delta_context));
	ctxt->repo = repo;
	ctxt->opts = opts;
	ctxt->old_src = old_src;
	ctxt->new_src = new_src;

	setup_xdiff_options(opts, &ctxt->xdiff_config, &ctxt->xdiff_params);
static void diff_delta_init_context_from_diff_list(
	diff_delta_context *ctxt,
	git_diff_list *diff)
	diff_delta_init_context(
		ctxt, diff->repo, &diff->opts, diff->old_src, diff->new_src);
}
static void diff_delta_unload(diff_delta_context *ctxt)
{
	ctxt->diffed = 0;
	if (ctxt->loaded) {
		release_content(&ctxt->delta->old_file, &ctxt->old_data, ctxt->old_blob);
		release_content(&ctxt->delta->new_file, &ctxt->new_data, ctxt->new_blob);
		ctxt->loaded = 0;
	}
	ctxt->delta = NULL;
	ctxt->prepped = 0;
}
static int diff_delta_prep(diff_delta_context *ctxt)
{
	int error;
	if (ctxt->prepped || !ctxt->delta)
		return 0;
	error = diff_delta_is_binary_by_attr(ctxt);
	ctxt->prepped = !error;
	return error;
}
static int diff_delta_load(diff_delta_context *ctxt)
{
	int error = 0;
	git_diff_delta *delta = ctxt->delta;
	bool check_if_unmodified = false;
	if (ctxt->loaded || !ctxt->delta)
		return 0;
	if (!ctxt->prepped && (error = diff_delta_prep(ctxt)) < 0)
		goto cleanup;
	ctxt->old_data.data = "";
	ctxt->old_data.len  = 0;
	ctxt->old_blob      = NULL;
	ctxt->new_data.data = "";
	ctxt->new_data.len  = 0;
	ctxt->new_blob      = NULL;
	if (delta->binary == 1)
		goto cleanup;
	switch (delta->status) {
	case GIT_DELTA_ADDED:
		delta->old_file.flags |= GIT_DIFF_FILE_NO_DATA;
		break;
	case GIT_DELTA_DELETED:
		delta->new_file.flags |= GIT_DIFF_FILE_NO_DATA;
		break;
	case GIT_DELTA_MODIFIED:
		break;
	default:
		delta->new_file.flags |= GIT_DIFF_FILE_NO_DATA;
		delta->old_file.flags |= GIT_DIFF_FILE_NO_DATA;
		break;
	}
#define CHECK_UNMODIFIED (GIT_DIFF_FILE_NO_DATA | GIT_DIFF_FILE_VALID_OID)
	check_if_unmodified =
		(delta->old_file.flags & CHECK_UNMODIFIED) == 0 &&
		(delta->new_file.flags & CHECK_UNMODIFIED) == 0;

	/* Always try to load workdir content first, since it may need to be
	 * filtered (and hence use 2x memory) and we want to minimize the max
	 * memory footprint during diff.
	 */

	if ((delta->old_file.flags & GIT_DIFF_FILE_NO_DATA) == 0 &&
		ctxt->old_src == GIT_ITERATOR_WORKDIR) {
		if ((error = get_workdir_content(
				ctxt, &delta->old_file, &ctxt->old_data)) < 0)
			goto cleanup;
		if (delta->binary == 1)
			goto cleanup;
	}
	if ((delta->new_file.flags & GIT_DIFF_FILE_NO_DATA) == 0 &&
		ctxt->new_src == GIT_ITERATOR_WORKDIR) {
		if ((error = get_workdir_content(
				ctxt, &delta->new_file, &ctxt->new_data)) < 0)
			goto cleanup;
	}
	if ((delta->old_file.flags & GIT_DIFF_FILE_NO_DATA) == 0 &&
		ctxt->old_src != GIT_ITERATOR_WORKDIR) {
		if ((error = get_blob_content(
				ctxt, &delta->old_file, &ctxt->old_data, &ctxt->old_blob)) < 0)
			goto cleanup;
		if (delta->binary == 1)
			goto cleanup;
	}

	if ((delta->new_file.flags & GIT_DIFF_FILE_NO_DATA) == 0 &&
		ctxt->new_src != GIT_ITERATOR_WORKDIR) {
		if ((error = get_blob_content(
				ctxt, &delta->new_file, &ctxt->new_data, &ctxt->new_blob)) < 0)
			goto cleanup;
		if (delta->binary == 1)
	}

	/* if we did not previously have the definitive oid, we may have
	 * incorrect status and need to switch this to UNMODIFIED.
	 */
	if (check_if_unmodified &&
		delta->old_file.mode == delta->new_file.mode &&
		!git_oid_cmp(&delta->old_file.oid, &delta->new_file.oid))
	{
		delta->status = GIT_DELTA_UNMODIFIED;
		if ((ctxt->opts->flags & GIT_DIFF_INCLUDE_UNMODIFIED) == 0)
	}

cleanup:
	/* last change to update binary flag */
	if (delta->binary == -1)
		update_delta_is_binary(delta);

	ctxt->loaded = !error;
	/* flag if we would want to diff the contents of these files */
	if (ctxt->loaded)
		ctxt->diffable =
			(delta->binary != 1 &&
			 delta->status != GIT_DELTA_UNMODIFIED &&
			 (ctxt->old_data.len || ctxt->new_data.len) &&
			 git_oid_cmp(&delta->old_file.oid, &delta->new_file.oid));
	return error;
}

static int diff_delta_cb(void *priv, mmbuffer_t *bufs, int len)
{
	diff_delta_context *ctxt = priv;
	if (len == 1) {
		if ((ctxt->cb_error = parse_hunk_header(&ctxt->range, bufs[0].ptr)) < 0)
			return ctxt->cb_error;

		if (ctxt->per_hunk != NULL &&
			ctxt->per_hunk(ctxt->cb_data, ctxt->delta, &ctxt->range,
				bufs[0].ptr, bufs[0].size))
			ctxt->cb_error = GIT_EUSER;
	}

	if (len == 2 || len == 3) {
		/* expect " "/"-"/"+", then data */
		char origin =
			(*bufs[0].ptr == '+') ? GIT_DIFF_LINE_ADDITION :
			(*bufs[0].ptr == '-') ? GIT_DIFF_LINE_DELETION :
			GIT_DIFF_LINE_CONTEXT;

		if (ctxt->per_line != NULL &&
			ctxt->per_line(ctxt->cb_data, ctxt->delta, &ctxt->range, origin,
				bufs[1].ptr, bufs[1].size))
			ctxt->cb_error = GIT_EUSER;
	}

	if (len == 3 && !ctxt->cb_error) {
		/* If we have a '+' and a third buf, then we have added a line
		 * without a newline and the old code had one, so DEL_EOFNL.
		 * If we have a '-' and a third buf, then we have removed a line
		 * with out a newline but added a blank line, so ADD_EOFNL.
		 */
		char origin =
			(*bufs[0].ptr == '+') ? GIT_DIFF_LINE_DEL_EOFNL :
			(*bufs[0].ptr == '-') ? GIT_DIFF_LINE_ADD_EOFNL :
			GIT_DIFF_LINE_CONTEXT;

		if (ctxt->per_line != NULL &&
			ctxt->per_line(ctxt->cb_data, ctxt->delta, &ctxt->range, origin,
				bufs[2].ptr, bufs[2].size))
			ctxt->cb_error = GIT_EUSER;
	}

	return ctxt->cb_error;
}

static int diff_delta_exec(
	diff_delta_context *ctxt,
	void *cb_data,
	git_diff_hunk_fn per_hunk,
	git_diff_data_fn per_line)
{
	int error = 0;
	xdemitcb_t xdiff_callback;
	mmfile_t old_xdiff_data, new_xdiff_data;

	if (ctxt->diffed || !ctxt->delta)
		return 0;

	if (!ctxt->loaded && (error = diff_delta_load(ctxt)) < 0)
		goto cleanup;

	if (!ctxt->diffable)
		return 0;

	ctxt->cb_data  = cb_data;
	ctxt->per_hunk = per_hunk;
	ctxt->per_line = per_line;
	ctxt->cb_error = 0;

	memset(&xdiff_callback, 0, sizeof(xdiff_callback));
	xdiff_callback.outf = diff_delta_cb;
	xdiff_callback.priv = ctxt;

	old_xdiff_data.ptr  = ctxt->old_data.data;
	old_xdiff_data.size = ctxt->old_data.len;
	new_xdiff_data.ptr  = ctxt->new_data.data;
	new_xdiff_data.size = ctxt->new_data.len;

	xdl_diff(&old_xdiff_data, &new_xdiff_data,
		&ctxt->xdiff_params, &ctxt->xdiff_config, &xdiff_callback);

	error = ctxt->cb_error;
	ctxt->diffed = !error;

	return error;
}

int git_diff_foreach(
	git_diff_list *diff,
	void *data,
	git_diff_file_fn file_cb,
	git_diff_hunk_fn hunk_cb,
	git_diff_data_fn line_cb)
{
	int error = 0;
	diff_delta_context ctxt;
	size_t idx;

	diff_delta_init_context_from_diff_list(&ctxt, diff);

	git_vector_foreach(&diff->deltas, idx, ctxt.delta) {
		if (diff_delta_is_ambiguous(ctxt.delta))
			if ((error = diff_delta_load(&ctxt)) < 0)
				goto cleanup;

		if (diff_delta_should_skip(ctxt.opts, ctxt.delta))
			continue;

		if ((error = diff_delta_load(&ctxt)) < 0)
			goto cleanup;

		if (file_cb != NULL &&
			file_cb(data, ctxt.delta, (float)idx / diff->deltas.length) != 0)
		{
			error = GIT_EUSER;
			goto cleanup;
		}

		error = diff_delta_exec(&ctxt, data, hunk_cb, line_cb);

cleanup:
		diff_delta_unload(&ctxt);
	if (error == GIT_EUSER)
		giterr_clear();

	if (pi->print_cb(pi->cb_data, delta, NULL, GIT_DIFF_LINE_FILE_HDR,
			git_buf_cstr(pi->buf), git_buf_len(pi->buf)))
	{
		giterr_clear();
		return GIT_EUSER;
	}

	return 0;
    if (pi->print_cb(pi->cb_data, delta, NULL, GIT_DIFF_LINE_FILE_HDR, git_buf_cstr(pi->buf), git_buf_len(pi->buf)))
	{
		giterr_clear();
		return GIT_EUSER;
	}
	if (pi->print_cb(pi->cb_data, delta, NULL, GIT_DIFF_LINE_BINARY,
			git_buf_cstr(pi->buf), git_buf_len(pi->buf)))
	{
		giterr_clear();
		return GIT_EUSER;
	}

	return 0;
	if (pi->print_cb(pi->cb_data, d, r, GIT_DIFF_LINE_HUNK_HDR,
			git_buf_cstr(pi->buf), git_buf_len(pi->buf)))
	{
		giterr_clear();
		return GIT_EUSER;
	}

	return 0;
	if (pi->print_cb(pi->cb_data, delta, range, line_origin,
			git_buf_cstr(pi->buf), git_buf_len(pi->buf)))
	{
		giterr_clear();
		return GIT_EUSER;
	}

	return 0;
int git_diff_entrycount(git_diff_list *diff, int delta_t)
{
	int count = 0;
	unsigned int i;
	git_diff_delta *delta;

	assert(diff);

	git_vector_foreach(&diff->deltas, i, delta) {
		if (diff_delta_should_skip(&diff->opts, delta))
			continue;

		if (delta_t < 0 || delta->status == (git_delta_t)delta_t)
			count++;
	}

	/* It is possible that this has overcounted the number of diffs because
	 * there may be entries that are marked as MODIFIED due to differences
	 * in stat() output that will turn out to be the same once we calculate
	 * the actual SHA of the data on disk.
	 */

	return count;
}

static void set_data_from_blob(
	git_blob *blob, git_map *map, git_diff_file *file)
{
	if (blob) {
		map->data = (char *)git_blob_rawcontent(blob);
		file->size = map->len = git_blob_rawsize(blob);
		git_oid_cpy(&file->oid, git_object_id((const git_object *)blob));
	} else {
		map->data = "";
		file->size = map->len = 0;
		file->flags |= GIT_DIFF_FILE_NO_DATA;
	}
}

	int error;
	diff_delta_context ctxt;
	git_repository *repo;
	if (new)
		repo = git_object_owner((git_object *)new);
	else if (old)
		repo = git_object_owner((git_object *)old);
	else
		repo = NULL;
	diff_delta_init_context(
		&ctxt, repo, options, GIT_ITERATOR_TREE, GIT_ITERATOR_TREE);

	memset(&delta, 0, sizeof(delta));

	set_data_from_blob(old, &ctxt.old_data, &delta.old_file);
	set_data_from_blob(new, &ctxt.new_data, &delta.new_file);

	ctxt.delta = &delta;
	if ((error = diff_delta_prep(&ctxt)) < 0)
		goto cleanup;
	if (delta.binary == -1) {
		if ((error = diff_delta_is_binary_by_content(
				&ctxt, &delta.old_file, &ctxt.old_data)) < 0 ||
			(error = diff_delta_is_binary_by_content(
				&ctxt, &delta.new_file, &ctxt.new_data)) < 0)
			goto cleanup;
	}

	ctxt.loaded = 1;
	ctxt.diffable = (delta.binary != 1 && delta.status != GIT_DELTA_UNMODIFIED);

	/* do diffs */

	if (file_cb != NULL && file_cb(cb_data, &delta, 1)) {
		error = GIT_EUSER;
		goto cleanup;
	}

	error = diff_delta_exec(&ctxt, cb_data, hunk_cb, line_cb);

cleanup:
	if (error == GIT_EUSER)
		giterr_clear();

	diff_delta_unload(&ctxt);

	return error;
}

typedef struct diffiter_line diffiter_line;
struct diffiter_line {
	diffiter_line *next;
	char origin;
	const char *ptr;
	size_t len;
};

typedef struct diffiter_hunk diffiter_hunk;
struct diffiter_hunk {
	diffiter_hunk *next;
	git_diff_range range;
	diffiter_line *line_head;
	size_t line_count;
};

struct git_diff_iterator {
	git_diff_list *diff;
	diff_delta_context ctxt;
	size_t file_index;
	size_t next_index;
	git_pool hunks;
	size_t   hunk_count;
	diffiter_hunk *hunk_head;
	diffiter_hunk *hunk_curr;
	char hunk_header[128];
	git_pool lines;
	diffiter_line *line_curr;
};

typedef struct {
	git_diff_iterator *iter;
	diffiter_hunk *last_hunk;
	diffiter_line *last_line;
} diffiter_cb_info;

static int diffiter_hunk_cb(
	void *cb_data,
	git_diff_delta *delta,
	git_diff_range *range,
	const char *header,
	size_t header_len)
{
	diffiter_cb_info *info = cb_data;
	git_diff_iterator *iter = info->iter;
	diffiter_hunk *hunk;

	GIT_UNUSED(delta);
	GIT_UNUSED(header);
	GIT_UNUSED(header_len);

	if ((hunk = git_pool_mallocz(&iter->hunks, 1)) == NULL) {
		iter->ctxt.cb_error = -1;
	}
	if (info->last_hunk)
		info->last_hunk->next = hunk;
	info->last_hunk = hunk;

	memcpy(&hunk->range, range, sizeof(hunk->range));

	iter->hunk_count++;

	if (iter->hunk_head == NULL)
		iter->hunk_curr = iter->hunk_head = hunk;

	return 0;
}

static int diffiter_line_cb(
	void *cb_data,
	git_diff_delta *delta,
	git_diff_range *range,
	char line_origin,
	const char *content,
	size_t content_len)
{
	diffiter_cb_info *info = cb_data;
	git_diff_iterator *iter = info->iter;
	diffiter_line *line;

	GIT_UNUSED(delta);
	GIT_UNUSED(range);

	if ((line = git_pool_mallocz(&iter->lines, 1)) == NULL) {
		iter->ctxt.cb_error = -1;
		return -1;
	if (info->last_line)
		info->last_line->next = line;
	info->last_line = line;
	line->origin = line_origin;
	line->ptr = content;
	line->len = content_len;

	info->last_hunk->line_count++;

	if (info->last_hunk->line_head == NULL)
		info->last_hunk->line_head = line;

	return 0;
}

static int diffiter_do_diff_file(git_diff_iterator *iter)
{
	int error;
	diffiter_cb_info info;

	if (iter->ctxt.diffed || !iter->ctxt.delta)
	memset(&info, 0, sizeof(info));
	info.iter = iter;
	error = diff_delta_exec(
		&iter->ctxt, &info, diffiter_hunk_cb, diffiter_line_cb);
	if (error == GIT_EUSER)
		error = iter->ctxt.cb_error;

	return error;
}

static void diffiter_do_unload_file(git_diff_iterator *iter)
{
	if (iter->ctxt.loaded) {
		diff_delta_unload(&iter->ctxt);

		git_pool_clear(&iter->lines);
		git_pool_clear(&iter->hunks);
	}

	iter->ctxt.delta = NULL;
	iter->hunk_head = NULL;
	iter->hunk_count = 0;
}

int git_diff_iterator_new(
	git_diff_iterator **iterator_ptr,
	git_diff_list *diff)
{
	git_diff_iterator *iter;

	assert(diff && iterator_ptr);

	*iterator_ptr = NULL;

	iter = git__malloc(sizeof(git_diff_iterator));
	GITERR_CHECK_ALLOC(iter);

	memset(iter, 0, sizeof(*iter));

	iter->diff = diff;
	GIT_REFCOUNT_INC(iter->diff);

	diff_delta_init_context_from_diff_list(&iter->ctxt, diff);

	if (git_pool_init(&iter->hunks, sizeof(diffiter_hunk), 0) < 0 ||
		git_pool_init(&iter->lines, sizeof(diffiter_line), 0) < 0)
		goto fail;

	*iterator_ptr = iter;

fail:
	git_diff_iterator_free(iter);

	return -1;
}

void git_diff_iterator_free(git_diff_iterator *iter)
{
	diffiter_do_unload_file(iter);
	git_diff_list_free(iter->diff); /* decrement ref count */
	git__free(iter);
}

float git_diff_iterator_progress(git_diff_iterator *iter)
{
	return (float)iter->next_index / (float)iter->diff->deltas.length;
}

int git_diff_iterator__max_files(git_diff_iterator *iter)
{
	return (int)iter->diff->deltas.length;
}

int git_diff_iterator_num_hunks_in_file(git_diff_iterator *iter)
{
	int error = diffiter_do_diff_file(iter);
	return (error != 0) ? error : (int)iter->hunk_count;
}

int git_diff_iterator_num_lines_in_hunk(git_diff_iterator *iter)
{
	int error = diffiter_do_diff_file(iter);
	if (!error && iter->hunk_curr)
		error = iter->hunk_curr->line_count;
	return error;
}

int git_diff_iterator_next_file(
	git_diff_delta **delta_ptr,
	git_diff_iterator *iter)
{
	int error = 0;

	assert(iter);

	iter->file_index = iter->next_index;

	diffiter_do_unload_file(iter);

	while (!error) {
		iter->ctxt.delta = git_vector_get(&iter->diff->deltas, iter->file_index);
		if (!iter->ctxt.delta) {
			error = GIT_ITEROVER;
			break;
		}

		if (diff_delta_is_ambiguous(iter->ctxt.delta) &&
			(error = diff_delta_load(&iter->ctxt)) < 0)
			break;

		if (!diff_delta_should_skip(iter->ctxt.opts, iter->ctxt.delta))
			break;

		iter->file_index++;
	}

	if (!error) {
		iter->next_index = iter->file_index + 1;

		error = diff_delta_prep(&iter->ctxt);
	}

	if (iter->ctxt.delta == NULL) {
		iter->hunk_curr = NULL;
		iter->line_curr = NULL;
	}

	if (delta_ptr != NULL)
		*delta_ptr = !error ? iter->ctxt.delta : NULL;

	return error;
}

int git_diff_iterator_next_hunk(
	git_diff_range **range_ptr,
	const char **header,
	size_t *header_len,
	git_diff_iterator *iter)
{
	int error = diffiter_do_diff_file(iter);
	git_diff_range *range;

	if (error)
		return error;

	if (iter->hunk_curr == NULL) {
		if (range_ptr) *range_ptr = NULL;
		if (header) *header = NULL;
		if (header_len) *header_len = 0;
		iter->line_curr = NULL;
		return GIT_ITEROVER;
	}

	range = &iter->hunk_curr->range;

	if (range_ptr)
		*range_ptr = range;

	if (header) {
		int out = format_hunk_header(
			iter->hunk_header, sizeof(iter->hunk_header), range);

		/* TODO: append function name to header */

		*(iter->hunk_header + out++) = '\n';

		*header = iter->hunk_header;

		if (header_len)
			*header_len = (size_t)out;
	}

	iter->line_curr = iter->hunk_curr->line_head;
	iter->hunk_curr = iter->hunk_curr->next;

	return error;
}

int git_diff_iterator_next_line(
	char *line_origin, /**< GIT_DIFF_LINE_... value from above */
	const char **content_ptr,
	size_t *content_len,
	git_diff_iterator *iter)
{
	int error = diffiter_do_diff_file(iter);

	if (error)
		return error;

	/* if the user has not called next_hunk yet, call it implicitly (OK?) */
	if (iter->hunk_curr == iter->hunk_head) {
		error = git_diff_iterator_next_hunk(NULL, NULL, NULL, iter);
		if (error)
			return error;
	}

	if (iter->line_curr == NULL) {
		if (line_origin) *line_origin = GIT_DIFF_LINE_CONTEXT;
		if (content_ptr) *content_ptr = NULL;
		if (content_len) *content_len = 0;
		return GIT_ITEROVER;
	}

	if (line_origin)
		*line_origin = iter->line_curr->origin;
	if (content_ptr)
		*content_ptr = iter->line_curr->ptr;
	if (content_len)
		*content_len = iter->line_curr->len;

	iter->line_curr = iter->line_curr->next;

	return error;