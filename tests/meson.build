# Test sources
test_src = files(
    'test_CommChannel.cc',
    'test_Context.cc',
    'test_exceptions.cc',
    'test_Executor.cc',
    #'tests/test_format.cc' needs fmt{} library
    'test_internals.cc',
    'test_LockedQueue.cc',
    'test_lua_details.cc',
    'test_main.cc',
    'test_Message.cc',
    'test_send_message.cc',
    'test_Sequence.cc',
    'test_SequenceManager.cc',
    'test_serialize_sequence.cc',
    'test_Step.cc',
    'test_time_types.cc',
    'test_Timeout.cc',
    'test_VariableName.cc',
    'test_GitWrapper.cc'
)

# The tests are executed in the build dir to avoid pollution of the git repository with
# the test executable's output files
test('all',
    executable(meson.project_name() + '_test',
        test_src,
        dependencies : taskolib_dep,
    ),
    workdir : meson.current_build_dir(),
    timeout : 10,
)

fmt_dep = dependency('fmt', required : false)
if fmt_dep.found()
    test('format',
        executable(meson.project_name() + '_format_test',
            [ 'test_format.cc', 'test_main.cc', ],
            dependencies : [ taskolib_dep, dependency('fmt'), ],
        ),
        timeout : 10,
    )
endif

######
# Test if all headers are self-contained (Core Guidelines SF.11)
# This is automated over all public_headers

message('Generating self-containment tests')
all_header_checks = [ ]
loop_number = 0
foreach header_to_check : public_headers
    loop_number += 1
    if header_to_check.endswith('.h')
        header_name = header_to_check.split('.h')[0]
        if not fmt_dep.found() and header_name.endswith('/format')
            continue
        endif

        header_conf = configuration_data()
        header_conf.set('HEADER_TO_INCLUDE', header_to_check)
        header_conf.set('DUMMY_FUNCTION_NAME', 'a@0@'.format(loop_number))

        header_check = configure_file(
            input : 'test_self_containment.cc.in',
            output : 'incl_' + header_name.split('/')[-1] + '.cc',
            configuration : header_conf,
        )
        all_header_checks += [ header_check ]
    endif
endforeach

static_library(
    '_ignore_me', all_header_checks,
    dependencies : taskolib_dep,
    install : false,
)
