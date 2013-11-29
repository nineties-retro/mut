#
# A simple (recursive) dependency generator for C files.
# This dependency generator can only cope with files in which _all_ #include
# files are meant to be included i.e. no conditional inclusion except to
# protect against symbol clashes.  This means it is completely useless
# for most C programs but works perfectly for my programs since I don't
# use #ifdef to control features -- see mut/doc/c/ifdef.feature.
# Also it only notices #include statements with no whitespace between the
# "#" and "include" -- easy to change but since I don't write includes like
# that I have little incentive.

function output_dependencies(file) {
    if (!files_seen[file]) {
        for (i = 1; i <= n_dirs; i += 1) {
            path = dirs[i]"/"file;
            if ((getline l < path) > 0) {
                printf(" %s", path);
                output_dependencies_aux(path, l);
                files_seen[file] = 1;
                break;
            }
        }
    }
}


function output_dependencies_aux(file, l) {
    do {
        split(l, lb);
        if (lb[1] == "#include") {
            if (substr(lb[2], 1, 1) == "\"") {
                fn = substr(lb[2], 2, length(lb[2])-2);
                output_dependencies(fn);
            }
        }
    } while ((getline l < file) > 0);
    close(file)
}

END {
    printf("%s %s: %s", object_file, dependency_file, input);
    n_dirs = split(include_path, dirs, ":");
    if ((getline l < input) > 0) {
        output_dependencies_aux(input, l);
    }
    printf("\n");
}
