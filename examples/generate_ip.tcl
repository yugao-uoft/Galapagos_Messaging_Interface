


open_project hls_project
set_top test_sender
open_solution test_sender
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog


open_project hls_project
set_top test_receiver
open_solution test_receiver
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top packetizer
open_solution packetizer
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top gather_recv_20
open_solution gather_recv_20
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top gather_recv_21
open_solution gather_recv_21
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top gather_recv_22
open_solution gather_recv_22
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top gather_send_20
open_solution gather_send_20
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top gather_send_21
open_solution gather_send_21
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top gather_send_22
open_solution gather_send_22
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top scatter_1
open_solution scatter_1
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top scatter_2
open_solution scatter_2
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top scatter_3
open_solution scatter_3
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog


open_project hls_project
set_top compute_4
open_solution compute_4
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_5
open_solution compute_5
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_6
open_solution compute_6
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_7
open_solution compute_7
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_8
open_solution compute_8
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_9
open_solution compute_9
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_10
open_solution compute_10
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_11
open_solution compute_11
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_12
open_solution compute_12
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_13
open_solution compute_13
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_14
open_solution compute_14
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_15
open_solution compute_15
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_16
open_solution compute_16
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_17
open_solution compute_17
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_18
open_solution compute_18
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog

open_project hls_project
set_top compute_19
open_solution compute_19
add_files src/top.cpp
add_files src/common.hpp
add_files src/parameters.hpp
set_part {xczu19eg-ffvc1760-2-i} -tool vivado
create_clock  -period 5.0 -name default
csynth_design
export_design -rtl verilog -format ip_catalog
