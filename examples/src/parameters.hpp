#pragma once
#include "common.hpp" 




struct config_t_compute_4
{
static const unsigned id=4;
static const unsigned dest=22;
};
struct config_t_compute_5
{
static const unsigned id=5;
static const unsigned dest=22;
};
struct config_t_compute_6
{
static const unsigned id=6;
static const unsigned dest=22;
};
struct config_t_compute_7
{
static const unsigned id=7;
static const unsigned dest=22;
};
struct config_t_compute_8
{
static const unsigned id=8;
static const unsigned dest=22;
};

struct config_t_compute_9
{
static const unsigned id=9;
static const unsigned dest=20;
};
struct config_t_compute_10
{
static const unsigned id=10;
static const unsigned dest=20;
};
struct config_t_compute_11
{
static const unsigned id=11;
static const unsigned dest=20;
};
struct config_t_compute_12
{
static const unsigned id=12;
static const unsigned dest=20;
};
struct config_t_compute_13
{
static const unsigned id=13;
static const unsigned dest=20;
};

struct config_t_compute_14
{
static const unsigned id=14;
static const unsigned dest=21;
};
struct config_t_compute_15
{
static const unsigned id=15;
static const unsigned dest=21;
};
struct config_t_compute_16
{
static const unsigned id=16;
static const unsigned dest=21;
};
struct config_t_compute_17
{
static const unsigned id=17;
static const unsigned dest=21;
};
struct config_t_compute_18
{
static const unsigned id=18;
static const unsigned dest=21;
};
struct config_t_compute_19
{
static const unsigned id=19;
static const unsigned dest=21;
};

struct config_t_gather_20
{
static const unsigned NUM_CHILDREN=5;
static const unsigned id=20;
static const unsigned dest=22;
static const unsigned base_src=9;
static const unsigned MAX_PACKET_LENGTH=16;
};

struct config_t_gather_21
{
static const unsigned NUM_CHILDREN=6;
static const unsigned id=21;
static const unsigned dest=22;
static const unsigned base_src=14;
static const unsigned MAX_PACKET_LENGTH=16;
};

struct config_t_gather_22
{
static const unsigned NUM_CHILDREN=5;
static const unsigned NUM_NODE=2;
static const unsigned NUM_FIFO=NUM_CHILDREN+NUM_NODE;
static const unsigned id=22;
static const unsigned dest=23;
static const unsigned child_id_start=4;
static const unsigned child_id_end=8;
static const unsigned node_id_start=20;
static const unsigned node_id_end=21;
static const unsigned MAX_PACKET_LENGTH=16;
};


struct config_t_scatter_1
{
static const unsigned id=1;
static const unsigned NUM_CHILDREN=5;
static const unsigned TOTAL_NUM_CHILDREN=16;
static const unsigned NUM_NODE=2;
static const unsigned MAX_PACKET_LENGTH=16;
static const unsigned LAST_NODE=0;
};

const ap_uint<PACKET_DEST_LENGTH> scatter_1_children_id[config_t_scatter_1::NUM_CHILDREN] = {4,5,6,7,8};
const ap_uint<PACKET_DEST_LENGTH> scatter_1_node_id[config_t_scatter_1::NUM_NODE] = {2,3};
const ap_uint<PACKET_DEST_LENGTH> scatter_1_num_children_per_node[config_t_scatter_1::NUM_NODE] = {5,6};


struct config_t_scatter_2
{
static const unsigned id=2;
static const unsigned NUM_CHILDREN=5;
static const unsigned TOTAL_NUM_CHILDREN=16;
static const unsigned MAX_PACKET_LENGTH=16;
static const unsigned LAST_NODE=0;
};

const ap_uint<PACKET_DEST_LENGTH> scatter_2_children_id[config_t_scatter_2::NUM_CHILDREN] = {9,10,11,12,13};

struct config_t_scatter_3
{
static const unsigned id=3;
static const unsigned NUM_CHILDREN=6;
static const unsigned TOTAL_NUM_CHILDREN=16;
static const unsigned MAX_PACKET_LENGTH=16;
static const unsigned LAST_NODE=1;
};

const ap_uint<PACKET_DEST_LENGTH> scatter_3_children_id[config_t_scatter_3::NUM_CHILDREN] = {14,15,16,17,18,19};

