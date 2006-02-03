#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"
#include "ipc.h"
#include "utils.h"
#include "record.h"

/* --------------------------------------------------------
 * PRIVATE: SHA1 message digest from OpenSSL 
 */
#include <openssl/evp.h>
#define DIGEST_STRING_MAX	(EVP_MAX_MD_SIZE * 2 + 1) // Two chars for each byte, plus one for NULL.

char *sha_file(const char *filename, char *result_ptr = NULL)
{
    EVP_MD_CTX ctx;
    unsigned int digest_len;
    char buf[STRING_MAX];

    static char result[DIGEST_STRING_MAX];

    if (result_ptr == NULL)
	result_ptr = result;

    FILE *fd = fopen(filename, "r");
    if (!fd) return NULL;

    EVP_DigestInit(&ctx, EVP_sha1());
    while (!feof(fd)) {
	unsigned long len = fread(buf, sizeof(char), sizeof(buf), fd);
	EVP_DigestUpdate(&ctx, buf, len);
    }
    EVP_DigestFinal(&ctx, (unsigned char *)buf, &digest_len);

    fclose(fd);

    for (unsigned int i = 0; i < digest_len; ++i)
	sprintf(&result_ptr[i*2], "%02x", (unsigned char)buf[i]);

    return result_ptr;
}

/* --------------------------------------------------------
 * PRIVATE: Given a binary filename, it will open a pipe to
 * `ldd', parse the output, and return a list of libraries
 */
bool get_libs(strlist *list, const char *prog)
{
    FILE *ldd_in;
    char *buf, *start, *end;

    buf = sprintf_static("ldd %s", prog);
    ldd_in = popen(buf, "r");

    while ( (buf = fgets_static(ldd_in)) != NULL) {
	if (strstr(buf, "needs:")) continue;

	start = strrchr(buf, '>');
	if (!start++) start = buf;
	while (isspace(*start)) ++start;

	end = strrchr(buf, '(');
	if (!end) end = buf + strlen(buf);
	while (start < end && isspace(*(end - 1))) --end;
	*end = '\0';

	strlist_insert(list, start);
    }

    pclose(ldd_in);
    return true;
}

strlist libs_diff(strlist *prev_run, strlist *curr_run)
{
    strlist result = STRLIST_INITIALIZER;
    char buf[STRING_MAX];

    unsigned prev_cnt = 0, curr_cnt = 0;
    char *prev_ptr = strlist_get(prev_run, prev_cnt);
    char *curr_ptr = strlist_get(curr_run, curr_cnt);
    int retval;

    while (prev_ptr || curr_ptr) {
	if (prev_ptr && curr_ptr) {
	    retval = strcmp(prev_ptr, curr_ptr);

	    if (retval == 0) {
		// Libraries match.  Compare file contents.
		char *prev_hash = strlist_get(prev_run, ++prev_cnt);
		char *curr_hash = strlist_get(curr_run, ++curr_cnt);

		if (strcmp(prev_hash, curr_hash) != 0) {
		    snprintf(buf, sizeof(buf), "* %s has changed since last run.", prev_ptr);
		    strlist_push_back(&result, buf);
		}

		prev_ptr = strlist_get(prev_run, ++prev_cnt);
		curr_ptr = strlist_get(curr_run, ++curr_cnt);
		continue;
	    }
	}

	if (!curr_ptr || retval < 0) {
	    // If (there are no more libraries from `ldd`) OR
	    //    (library from history record not found in `ldd` list)

	    snprintf(buf, sizeof(buf), "* Mutatee is no longer dependent on %s.", prev_ptr);
	    strlist_push_back(&result, buf);

	    ++prev_cnt; // Skip digest string.
	    prev_ptr = strlist_get(prev_run, ++prev_cnt);

	} else if (!prev_ptr || retval > 0) {
	    // If (there are no more libraries from the history record) OR
	    //    (library from `ldd` not found in history record list)

	    snprintf(buf, sizeof(buf), "* Mutatee is now dependent on %s.", curr_ptr);
	    strlist_push_back(&result, buf);

	    ++curr_cnt; // Skip digest string.
	    curr_ptr = strlist_get(curr_run, ++curr_cnt);
	}
    }
    return result;
}

/* --------------------------------------------------------
 * PUBLIC: Implementation of public history record functions.
 */

record_t record_init()
{
    record_t result = { false, STRLIST_INITIALIZER, STRLIST_INITIALIZER, {0}, NULL, NULL };
    return result;
}

void record_clear(record_t *record)
{
    record->enabled = false;
    strlist_clear(&record->prog_line);
    strlist_clear(&record->lib_line);
    record->filename[0] = '\0';
    record->fd = NULL;
    record->raw_fd = NULL;
}

bool record_create(record_t *record, const char *prog, int argc, char **argv)
{
    char hash_buf[DIGEST_STRING_MAX];
    const char *prog_bin = prog;
    unsigned i;

    record_clear(record);

    if (strrchr(prog, '/'))
	prog_bin = strrchr(prog, '/') + 1;

    if (!sha_file(prog, hash_buf)) return false;
    strlist_push_back(&record->prog_line, prog_bin);
    strlist_push_back(&record->prog_line, sha_file(prog, hash_buf));
    for (i = 0; i < (unsigned)argc; ++i)
	strlist_push_back(&record->prog_line, argv[i]);

    strlist lib_list = STRLIST_INITIALIZER;
    get_libs(&lib_list, prog);
    for (i = 0; i < lib_list.count; ++i) {
	const char *lib_path = strlist_get(&lib_list, i);

	if (!sha_file(lib_path, hash_buf)) continue;
	strlist_push_back(&record->lib_line, lib_path);
	strlist_push_back(&record->lib_line, sha_file(lib_path, hash_buf));
    }

    record->enabled = true;
    return true;
}

bool record_search(record_t *newRecord)
{
    FILE *index_fd;
    char *index_filename;
    bool found = false, latest = true;

    index_filename = sprintf_static("%s/%s", config.record_dir, HISTORY_RECORD_INDEX_FILE);
    errno = 0;
    index_fd = fopen(index_filename, "r");

    if (errno && errno == ENOENT) {
	index_fd = fopen(index_filename, "w");
	if (!index_fd) {
	    fprintf(config.outfd, "*\n* Error opening history record index %s.  Disabling history record.\n*\n", index_filename);
	    newRecord->enabled = false;
	    return false;
	}
	fclose(index_fd);

    } else if (!index_fd) {
	fprintf(config.outfd, "*\n* Error opening history record index %s.  Disabling history record.\n*\n", index_filename);
        newRecord->enabled = false;
	return false;

    } else {
	// We should use C++ string instead to avoid buffer overflows.
	char *buf;
	while ( (buf = fgets_static(index_fd))) {
	    if (buf[0] == '\t' || buf[0] == '\n') continue;

	    chomp(buf);
	    strlist prog_line = char2strlist(buf);
	    if (strlist_cmp(&prog_line, &newRecord->prog_line)) {

		strlist diff_list = STRLIST_INITIALIZER;
		while (!found && (buf = fgets_static(index_fd)) && buf[0] == '\t') {
		    chomp(buf);
		    strlist lib_line = char2strlist(buf + 1);

		    strncpy(newRecord->filename, strlist_get(&lib_line, 0), sizeof(newRecord->filename));
		    strlist_pop_front(&lib_line);

		    if (strlist_cmp(&lib_line, &newRecord->lib_line)) {
			found = true;

		    } else {
			if (latest)
			    diff_list = libs_diff(&lib_line, &newRecord->lib_line);
			latest = false;
		    }
		    strlist_clear(&lib_line);
		}

		if (!found) {
		    fprintf(config.outfd, "* Warning: Current library set unknown.  Differences from latest run include:\n");
		    fprintf(config.outfd, "*\n");
		    for (unsigned i = 0; i < diff_list.count; ++i)
			fprintf(config.outfd, "%s\n", strlist_get(&diff_list, i));

		} else if (!latest)
		    fprintf(config.outfd, "* Warning: Current library signature does not match latest run.\n");

		strlist_clear(&diff_list);
	    }
	    strlist_clear(&prog_line);
	}
	fclose(index_fd);
    }

    if (!found) {
	// Fill storage file information
	const char *prog_file = strlist_get(&newRecord->prog_line, 0);
	if (strrchr(prog_file, '/'))
	    prog_file = strrchr(prog_file, '/') + 1;

	snprintf(newRecord->filename, sizeof(newRecord->filename), "%s/%s-XXXXXX", config.record_dir, prog_file);
	int file_desc = mkstemp(newRecord->filename);
	newRecord->fd = fdopen(file_desc, "a");

    } else {
	newRecord->fd = fopen(newRecord->filename, "a");
    }

    char timestr[STRING_MAX];
    time_t timestamp = time(NULL);

    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %Z", localtime(&timestamp));
    fprintf(newRecord->fd, "Log start: %s\n", timestr);
    fprintf(newRecord->fd, "--------------------------------------------------------------------------------\n");

    char raw_filename[ PATH_MAX ];
    strncpy(raw_filename, newRecord->filename, sizeof(raw_filename));
    strncat(raw_filename, ".raw", sizeof(raw_filename) - strlen(raw_filename));
    newRecord->raw_fd = fopen(raw_filename, "a");

    fprintf(newRecord->raw_fd, "Raw transcript log start: %s\n", timestr);
    fprintf(newRecord->raw_fd, "--------------------------------------------------------------------------------\n");

    return (found && latest);
}

void record_update(record_t *newRecord)
{
    FILE *index_fd;
    FILE *new_fd;
    char index_filename[PATH_MAX];
    char new_index_filename[PATH_MAX];

    strncpy(index_filename, config.record_dir, sizeof(index_filename));
    strncat(index_filename, "/", sizeof(index_filename) - strlen(index_filename));
    strncat(index_filename, HISTORY_RECORD_INDEX_FILE, sizeof(index_filename) - strlen(index_filename));
    errno = 0;
    index_fd = fopen(index_filename, "r");
    if (!index_fd) {
	fprintf(stderr, "Could not open %s\n", index_filename);
	return;
    }

    strncpy(new_index_filename, index_filename, sizeof(new_index_filename));
    strncat(new_index_filename, ".update", sizeof(new_index_filename) - strlen(new_index_filename));
    new_fd = fopen(new_index_filename, "w");
    if (!new_fd) {
	fprintf(stderr, "Could not open %s\n", new_index_filename);
	return;
    }

    char buf[8196];
    bool found = false;
    strlist prog_line = STRLIST_INITIALIZER;
    while (fgets(buf, sizeof(buf), index_fd)) {
	if (buf[0] == '\t' || buf[0] == '\n') {
	    fprintf(new_fd, "%s", buf);  // NOTE: Do not print "%s\n" here.
	    buf[0] = '\0';
	    continue;
	}

	if (buf[ strlen(buf) - 1 ] == '\n')
	    buf[ strlen(buf) - 1 ] = '\0';
	prog_line = char2strlist(buf);
	if (strlist_cmp(&prog_line, &newRecord->prog_line)) {
	    fprintf(new_fd, "%s\n", buf);

	    strlist_push_front(&newRecord->lib_line, newRecord->filename);
	    strlist2char(&newRecord->lib_line, buf);
	    fprintf(new_fd, "\t%s\n", buf);
	    strlist_pop_front(&newRecord->lib_line);

	    while (fgets(buf, sizeof(buf), index_fd) && buf[0] == '\t') {
		if (buf[ strlen(buf) - 1 ] == '\n')
		    buf[ strlen(buf) - 1 ] = '\0';

		strlist lib_line = char2strlist(buf);
		strlist_pop_front(&lib_line);

		if (!strlist_cmp(&lib_line, &newRecord->lib_line))
		    fprintf(new_fd, "%s\n", buf);

		strlist_clear(&lib_line);
	    }
	    fprintf(new_fd, "%s", buf);  // NOTE: Do not print "%s\n" here.

	    found = true;
	    break;
	}

	if (strcmp(strlist_get(&newRecord->prog_line, 0),
		   strlist_get(&prog_line, 0)) < 0)
	    break;

	strlist_clear(&prog_line);
	fprintf(new_fd, "%s\n", buf);
	buf[0] = '\0';
    }

    // New binary signature entry.
    if (!found) {
	char buf2[8196];

	strlist2char(&newRecord->prog_line, buf2);
	fprintf(new_fd, "%s\n", buf2);

	strlist_push_front(&newRecord->lib_line, newRecord->filename);
	strlist2char(&newRecord->lib_line, buf2);
	fprintf(new_fd, "\t%s\n\n", buf2);
	strlist_pop_front(&newRecord->lib_line);

	if (buf[0] != '\0')
	    fprintf(new_fd, "%s\n", buf);
    }

    while (fgets(buf, sizeof(buf), index_fd))
	fprintf(new_fd, "%s", buf);  // NOTE: Do not print "%s\n" here.

    fclose(index_fd);
    fclose(new_fd);

    unlink(index_filename);
    rename(new_index_filename, index_filename);
}
