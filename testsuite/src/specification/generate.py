import os
import tuples
import utils
import test_info_new_gen
import cmake_mutatees
import cmake_mutators
import group_boilerplate


def generate(directory):
    info = {}
    tuples.read_tuples(directory + '/tuples.gz', info)
    platform = utils.find_platform(os.environ.get('PLATFORM'), info)

    test_info_new_gen.write_test_info_new_gen(directory, info, platform)
    cmake_mutatees.write_mutatee_cmakelists(directory, info, platform)
    cmake_mutators.write_mutator_cmakelists(directory, info, platform)
    group_boilerplate.write_group_mutatee_boilerplate(directory, info, platform)
