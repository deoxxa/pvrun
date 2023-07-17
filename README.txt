pvrun version 1.0.0
copyright 2023 conrad pankoff <deoxxa@fknsrs.biz> (https://www.fknsrs.biz/)

usage: ./src/pvrun [-h/-V] [OPTIONS... --] PROGRAM [ARGUMENTS]
  -h             print this help and exit (must be first argument)
  -V             print version and exit (must be first argument)
  OPTIONS        additional arguments to supply to pv (optional). must
                 begin with an argument starting with '-', must be
                 terminated with --, can not contain options -d or -R,
                 maximum count of 50, maximum combined length of 8192
                 bytes including spaces. if a positional 'file' argument
                 is supplied, it will not work as expected. see 'man pv'
                 for more information.
  PROGRAM        program to run (required)
  ARGUMENTS      arguments for target program (optional)

examples:
  pvrun -h
  pvrun -V
  pvrun cp src_file dst_file
  pvrun -- cp -a src_dir dst_dir
  pvrun -N my-copy -- cp -a src_dir dst_dir
