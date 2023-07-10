#ifndef ROSE_FileSystem_H
#define ROSE_FileSystem_H

#include <string>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <vector>

namespace rose {

/** Functions for operating on files in a filesystem. */
    namespace FileSystem {

/** Pattern to use when creating temporary files. */
        extern const char *tempNamePattern;

/** Name of entities in a filesystem. */
        typedef boost::filesystem::path Path;

/** Iterate over directory contents non-recursively. */
        typedef boost::filesystem::directory_iterator DirectoryIterator;

/** Iterate recursively into subdirectories. */
        typedef boost::filesystem::recursive_directory_iterator RecursiveDirectoryIterator;

/** Predicate returning true if path exists. */
        bool isExisting(const Path &path);

/** Predicate returning true for existing regular files. */
        bool isFile(const Path &path);

/** Predicate returning true for existing directories. */
        bool isDirectory(const Path &path);

/** Predicate returning true for existing symbolic links. */
        bool isSymbolicLink(const Path &path);

/** Predicate returning inverse of @ref isSymbolicLink. */
        bool isNotSymbolicLink(const Path &path);

/** Predicate returning true for matching names.
 *
 *  Returns true if and only if the final component of the path matches the specified regular expression.
 *
 *  For example, to find all files whose base name matches the glob "rose_*" use this (note that the corresponding regular
 *  expression is "rose_.*", with a dot):
 *
 * @code
 *  using namespace rose::FileSystem;
 *  Path top = "/foo/bar"; // where the search starts
 *  std::vector<Path> roseFiles = findAllNames(top, baseNameMatches(boost::regex("rose_.*")));
 * @endcode */
        class baseNameMatches {
            const boost::regex &re_;
        public:
            baseNameMatches(const boost::regex &re) : re_(re) { }

            bool operator()(const Path &path);
        };

/** Create a temporary directory.
 *
 *  The temporary directory is created as a subdirectory of the directory which is suitable for temporary files under the
 *  conventions of the operating system.  The specifics of how this path is determined are implementation defined (see
 *  <code>boost::filesystem::temp_directory_path</code>).  The created subdirectory has a name of the form
 *  "rose-%%%%%%%%-%%%%%%%%" where each "%" is a random hexadecimal digit.  Returns the path to this directory. */
        Path createTemporaryDirectory();

/** Normalize a path name.
 *
 *  Normalizes a path by removing "." and ".." components to the extent which is possible.
 *
 *  For instance, a name like "/foo/bar/../baz" will become "/foo/baz" and the name "/foo/./baz" will become
 *  "/foo/baz". However, the names "/../foo" and "./foo" cannot be changed because removing the ".." in the first case would
 *  place it in a different directory if the name were appended to another name, and in the second case it would convert a
 *  relative name to an absolute name. */
        Path makeNormal(const Path &);

/** Make path relative.
 *
 *  Makes the specified path relative to another path or the current working directory. */
        Path makeRelative(const Path &path, const Path &root = boost::filesystem::current_path());

/** Make path absolute.
 *
 *  Makes the specified path an absolute path if it is a relative path.  If relative, then assume @p root is what the path is
 *  relative to. */
        Path makeAbsolute(const Path &path, const Path &root = boost::filesystem::current_path());

/** Entries within a directory.
 *
 *  Returns a list of entries in a directory--the contents of a directory--without recursing into subdirectories. The return
 *  value is a sorted list of paths, each of which contains @p root as a prefix.  If a @p select predicate is supplied then
 *  only paths for which the predicate returns true become part of the return value. The predicate is called with the path that
 *  would become part of the return value. The @p root itself is never returned and never tested by the predicate.
 *
 *  If @p select is not specified then all entries are returned.
 *
 * @{ */
        template<class Select>
        std::vector <Path> findNames(const Path &root, Select select) {
            std::vector <Path> matching;
            if (isDirectory(root)) {
                for (DirectoryIterator iter(root); iter != DirectoryIterator(); ++iter) {
                    if (select(iter->path()))
                        matching.push_back(iter->path());
                }
            }
            std::sort(matching.begin(), matching.end());
            return matching;
        }

        std::vector <Path> findNames(const Path &root);
/** @} */

/** Recursive list of names satisfying predicate.
 *
 *  Returns a list of entries in a directory and all subdirectories recursively.  The return value is a sorted list of
 *  paths, each of which contains @p root as a prefix.  If a @p select predicate is supplied then only paths for which the
 *  predicate returns true become part of the return value.  If a @p descend predicate is supplied then this algorithm only
 *  recurses into subdirectories for which @p descend returns true.  The predicates are called with the path that would become
 *  part of the return value.  The @p root itself is never returned and never tested by the @p select or @p descend predicates.
 *
 *  If @p select is not specified then all entries are returned. If @p descend is not specified then the algorithm traverses
 *  into all subdirectories.  Symbolic links to directories are never followed, but are returned if the @p select predicate
 *  allows them.
 *
 * @{ */
        template<class Select, class Descend>
        std::vector <Path> findNamesRecursively(const Path &root, Select select, Descend descend) {
            std::vector <Path> matching;
            RecursiveDirectoryIterator end;
            for (RecursiveDirectoryIterator dentry(root); dentry != end; ++dentry) {
                if (select(dentry->path()))
                    matching.push_back(dentry->path());
                if (!descend(dentry->path()))
                    dentry.no_push();
            }
            std::sort(matching.begin(), matching.end());
            return matching;
        }

        template<class Select>
        std::vector <Path> findNamesRecursively(const Path &root, Select select) {
            return findNamesRecursively(root, select, isDirectory);
        }

        std::vector <Path> findNamesRecursively(const Path &root);
/** @} */

/** Copy a file.
 *
 *  Copies the contents of the source file to the destination file, overwriting the destination file if it existed. */
        void copyFile(const Path &sourceFileName, const Path &destinationFileName);

/** Copy files from one directory to another.
 *
 *  Each of the specified files are copied from their location under @p root to a similar location under @p
 *  destinationDirectory. Subdirectories of the destination directory are created as necessary.
 *
 *  Any file whose name is outside the @p root directory will similarly be created outside the @p destinationDirectory.
 *  For instance, copyFiles(["bar/baz"], "foo", "frob") will copy "bar/baz" to "frob/../bar/baz" since "bar" is apparently
 *  a sibling of "foo", and therefore must be a sibling of "frob".
 *
 *  Throws a <code>boost::filesystem::filesystem_error</code> on failure. */
        void copyFiles(const std::vector <Path> &files, const Path &root, const Path &destinationDirectory);

/** Recursively copy files.
 *
 *  Get a list of files by recursively matching files under @p root and then copy them to similar locations relative to @p
 *  destination. The @p root and @p destination must not overlap.  The @p select and @p descend arguments are the same as
 *  for the @ref findAllNames method. */
        template<class Select, class Descend>
        void copyFilesRecursively(const Path &root, const Path &destination, Select select, Descend descend) {
            std::vector <Path> files = findNamesRecursively(root, select, descend);
            files.erase(files.begin(), std::remove_if(files.begin(), files.end(), isFile)); // keep only isFile names
            copyFiles(files, root, destination);
        }

/** Return a list of all rose_* files */
        std::vector <Path> findRoseFilesRecursively(const Path &root);

/** Convert a path to a string.
 *
 *  Try not to use this.  Paths contain more information than std::string and the conversion may loose that info. */
        std::string toString(const Path &);


    } // namespace
} // namespace

#endif
