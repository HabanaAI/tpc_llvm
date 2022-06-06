## Sample usage: generate_intrinsics_doc.sh build_tpc_intrinsics_doc --greco --pdf --html

build_tpc_intrinsics_doc()
{
    local __working_dir=$PWD
    local __script_dir=`cd $(dirname $0) && pwd`
    local __xml_path=$__working_dir/xml
    local __rst_path=$__working_dir/rst
    local __intrinsic_macro=""
    local __chip=""
    local __intrinsics_path=$TPC_LLVM_ROOT/../clang/lib/Headers/tpc-intrinsics.h
    local __gen_pdf="yes"
    local __gen_html="no"
    local __venv_conf="yes"

    # parameter while-loop
    while [ -n "$1" ];
    do
        case $1 in
        --gaudi )
            __intrinsic_macro="\"__gaudi__=1\" \"__gaudi_plus__=1\""
            __chip="gaudi"
            ;;
        --gaudi2 )
            __intrinsic_macro="\"__gaudi2__=1\" \"__gaudi2_plus__=1\""
            __chip="gaudi2"
            ;;
        --doron1 )
            __intrinsic_macro="\"__doron1__=1\" \"__doron1_plus__=1\""
            __chip="doron1"
            ;;
        --greco )
            __intrinsic_macro="\"__greco__=1\" \"__gaudi_plus__=1\" \"__greco_plus__=1\""
            __chip="greco"
            ;;
        --pdf )
            __gen_pdf="yes"
            ;;
        --html )
            __gen_html="yes"
            ;;
        --no_venv_conf )
            __venv_conf="no"
            ;;
        *)
            echo "The parameter $1 is not allowed"
            usage $__scriptname
            return 1 # error
            ;;
        esac
        shift
    done

    ## Check which version of python is installed
    if [[ ( `python3 -V` = *'3.8'* ) ]]; then
        __python_ver="3.8"
    elif [[ `python3 -V` = *'3.7'* ]]; then
        __python_ver="3.7"
    else
        __python_ver="3.6"
    fi

    export __python_cmd="python${__python_ver}"
    echo "python version: $__python_cmd"

    ## Checks if python virtual env exists or not. If not then create an env for installing the python dependencies
    if ( ! test -f $__working_dir/.llvm_doc_venv/bin/activate ); then
        $__python_cmd -m venv $__working_dir/.llvm_doc_venv
        if [ $__venv_conf == "yes" ]; then
            cat << EOF > $__working_dir/.llvm_doc_venv/pip.conf
[global]
index =  https://artifactory-kfs.habana-labs.com/artifactory/api/pypi/pypi-virtual
index-url = https://artifactory-kfs.habana-labs.com/artifactory/api/pypi/pypi-virtual/simple
trusted-host = artifactory-kfs.habana-labs.com
EOF
        fi
    fi

    local __venv_path=$__working_dir/.llvm_doc_venv
    echo "virtual enviornment being used: $__venv_path"

    # ## Check which version of pip is installed
    if command -v $__venv_path/bin/pip3.8 >/dev/null 2>&1; then
        __pip_cmd="pip3.8"
    elif command -v $__venv_path/bin/pip3.7 >/dev/null 2>&1; then
        __pip_cmd="pip3.7"
    elif command -v $__venv_path/bin/pip3.6 >/dev/null 2>&1; then
        __pip_cmd="pip3.6"
    elif command -v $__venv_path/bin/pip3 >/dev/null 2>&1; then
        __pip_cmd="pip3"
    elif command -v $__venv_path/bin/python3 >/dev/null 2>&1; then
        __pip_cmd="${__python_cmd} -m pip"
    else
        echo "No pip3 installation found. Please install and run script again."
        __pip_cmd="NONE"
    fi
    export __pip_cmd
    echo "pip version: $__pip_cmd"

    ## Install the required packages
    $__venv_path/bin/$__pip_cmd install -r $__script_dir/requirements.txt

    # cd $__working_dir

    ## Remove the previously generated intermediate and final outputs
    rm -rf $__xml_path $__rst_path
    cp -r $__script_dir/rst $__working_dir/

    ## Run the doxygen to generate the xml output
    echo "PREDEFINED MACROS: $__intrinsic_macro"
    echo "INTRINSICS PATH: $__intrinsics_path"
    XML_PATH=$__xml_path INTRINSICS_PATH=$__intrinsics_path CHIP_MACRO=$__intrinsic_macro doxygen $__script_dir/doxygen.conf
    echo "xml output: $__xml_path" 

    ## Run xmlToRst.py to parse xml output of doxygen into rst format
    $__venv_path/bin/$__python_cmd $__script_dir/xmlToRst.py $__chip
    echo "rst output: $__rst_path"

    ## Run sphinx in order to generate pdf or html as required
    if [ -n "$__gen_pdf" ]; then
        rm -rf $__working_dir/$__chip/pdf
        $__venv_path/bin/sphinx-build -b pdf $__rst_path $__working_dir/$__chip/pdf
    fi
    if [ -n "$__gen_html" ]; then
        rm -rf $__working_dir/$__chip/html
        $__venv_path/bin/sphinx-build -b html $__rst_path $__working_dir/$__chip/html
    fi
}

"$@"

