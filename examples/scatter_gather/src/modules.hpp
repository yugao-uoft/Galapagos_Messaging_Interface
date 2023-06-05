#pragma once
#include "common.hpp"
#include "hls_math.h"

template<typename config_t>
void GMISkew(
		hls::stream<dataword>& in,
		hls::stream<dataword>& out)
{
#pragma HLS inline
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	enum state {IDLE=0, SEND_PACKET, SEND_PACKET_LAST_FLIT};

	static enum state sinkstate = IDLE;
	static dataword temp;
	static dataword temp_old;
	static dataword out_data;

	switch(sinkstate)
	{
	case IDLE:
		if (!in.empty() && !out.full())
		{
			temp = in.read();
			out_data.data.range(7,0) = config_t::virtual_dest; // port
			out_data.data.range(511,8) = temp.data.range(503,0);
			out_data.dest = config_t::dest;
			out_data.id = temp.id;
			out_data.last = 0;
			out_data.user = temp.user;
			temp_old = temp;
			out.write(out_data);

			if (temp.last)
			{
				sinkstate = SEND_PACKET_LAST_FLIT;
			}
			else
			{
				sinkstate = SEND_PACKET;
			}
		}
		break;
	case SEND_PACKET:
		if (!in.empty() && !out.full())
		{
			temp = in.read();
			out_data.data.range(511,8) = temp.data.range(503,0);
			out_data.data.range(7,0) = temp_old.data.range(511,504);
			out_data.last = 0;
			temp_old = temp;
			out.write(out_data);

			if (temp.last)
			{
				sinkstate = SEND_PACKET_LAST_FLIT;
			}
			else
			{
				sinkstate = SEND_PACKET;
			}
		}
		break;
	case SEND_PACKET_LAST_FLIT:
		if (!out.full())
		{
			out_data.data.range(511,8) = 0;
			out_data.data.range(7,0) = temp_old.data.range(511,504);
			out_data.last = 1;
			out.write(out_data);
			sinkstate = IDLE;
		}
		break;
	}
}

template<typename config_t>
void PacketDecoder(
		hls::stream<dataword>& in,
		hls::stream<dataword> out[config_t::NUM_OP_CODE]
		)
{
#pragma HLS inline
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	enum state {IDLE=0, DECODER_SEND_HEADER, DECODER_SEND_PACKET};

	static enum state sinkstate = IDLE;
	static dataword temp;
	static dataword temp_old;
	static dataword out_data;
	static ap_uint<8> op_code;

	switch(sinkstate)
	{
	case IDLE:
		if (!in.empty())
		{
			temp = in.read();
			op_code = temp.data.range(7,0) - config_t::base_id;
			out_data.data.range(503,0) = temp.data.range(511,8);
			out_data.dest = temp.dest;
			out_data.id = temp.id;
			out_data.last = 0;
			out_data.user = temp.user;
			sinkstate = DECODER_SEND_HEADER;
		}
		break;
	case DECODER_SEND_HEADER:
		if (!in.empty() && !out[op_code].full())
		{
			temp = in.read();
			out_data.data.range(511,504) = temp.data.range(7,0);
			temp_old = temp;
			out_data.last = temp.last;
			out[op_code].write(out_data);

			if (temp.last)
			{
				sinkstate = IDLE;
			}
			else
			{
				sinkstate = DECODER_SEND_PACKET;
			}
		}
		break;
	case DECODER_SEND_PACKET:
		if (!in.empty() && !out[op_code].full())
		{
			temp = in.read();
			out_data.data.range(511,504) = temp.data.range(7,0);
			out_data.data.range(503,0) = temp_old.data.range(511,8);
			temp_old = temp;
			out_data.last = temp.last;
			out[op_code].write(out_data);

			if (temp.last)
			{
				sinkstate = IDLE;
			}
			else
			{
				sinkstate = DECODER_SEND_PACKET;
			}
		}
		break;
	default:
		break;
	}
}



template<typename config_t>
void GatherRecvLeaf(
		hls::stream<dataword>& in,
		hls::stream<dataword> out[config_t::NUM_CHILDREN])
{
#pragma HLS inline
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE axis port=in

	dataword temp;
	ap_uint<1> not_full = 1;
	for (int i=0; i<config_t::NUM_CHILDREN; i++) {
		#pragma HLS unroll
		not_full &= ~out[i].full();
	}

	if (!in.empty() && not_full)
	{
		temp = in.read();
		out[temp.id - config_t::base_src].write(temp);
	}
}

template<typename config_t>
void GatherSendLeaf(
		hls::stream<dataword> in[config_t::NUM_CHILDREN],
		hls::stream<dataword>& out)
{
#pragma HLS inline
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE axis port=in

	enum state {IDLE=0, SEND_PACKET_FIRST_FLIT, SEND_PACKET};

	static dataword temp;
	static dataword first_flit_per_child[config_t::NUM_CHILDREN];
	static ap_uint<16> num_flit_per_child[config_t::NUM_CHILDREN];
	static ap_uint<16> total_num_flit;
	static state sinkstate = IDLE;
	static ap_uint<8> index;
	static ap_uint<16> flit_count;
	static ap_uint<16> total_flit_count;
	static ap_uint<16> message_length;

#pragma HLS array_partition variable=first_flit_per_child dim=0 complete
#pragma HLS array_partition variable=num_flit_per_child dim=0 complete


	switch(sinkstate)
	{
	case IDLE:
		{
			ap_uint<1> not_empty = 1;
			for (int i=0; i<config_t::NUM_CHILDREN; i++) {
				#pragma HLS unroll
				not_empty &= ~in[i].empty();
			}

			if (not_empty && !out.full())
			{
				for (int i=0; i<config_t::NUM_CHILDREN; i++) {
					#pragma HLS unroll
					first_flit_per_child[i] = in[i].read();
				}

				for (int i=0; i<config_t::NUM_CHILDREN; i++) {
					#pragma HLS unroll

					if (first_flit_per_child[i].user % PACKET_NUM_BYTES == 0)
					{
						num_flit_per_child[i] = first_flit_per_child[i].user / PACKET_NUM_BYTES;
					}
					else
					{
						num_flit_per_child[i] = first_flit_per_child[i].user / PACKET_NUM_BYTES + 1;
					}
				}

				total_num_flit = 0;
				for (int i=0; i<config_t::NUM_CHILDREN; i++) {
					#pragma HLS unroll
					total_num_flit += num_flit_per_child[i];
				}

				message_length = 0;
				for (int i=0; i<config_t::NUM_CHILDREN; i++) {
					#pragma HLS unroll
					message_length += first_flit_per_child[i].user;
				}

				index = 0;
				flit_count = 0;
				total_flit_count = 0;

				sinkstate = SEND_PACKET_FIRST_FLIT;
			}
		}
		break;
	case SEND_PACKET_FIRST_FLIT:
		if (!out.full())
		{
			temp.data = first_flit_per_child[index].data;

			if (total_flit_count % config_t::MAX_PACKET_LENGTH == config_t::MAX_PACKET_LENGTH-1)
			{
				temp.last = 1;
			}
			else if (total_flit_count == total_num_flit - 1)
			{
				temp.last = 1;
			}
			else
			{
				temp.last = 0;
			}

			temp.id = config_t::id;
			temp.dest = config_t::dest;
			temp.user = message_length;
			out.write(temp);

			if (flit_count == num_flit_per_child[index] - 1 && index == config_t::NUM_CHILDREN - 1)
			{
				total_flit_count = 0;
				flit_count = 0;
				index = 0;
				sinkstate = IDLE;
			}
			else if (flit_count == num_flit_per_child[index] - 1)
			{
				total_flit_count += 1;
				flit_count = 0;
				index += 1;
				sinkstate = SEND_PACKET_FIRST_FLIT;
			}
			else
			{
				total_flit_count += 1;
				flit_count += 1;
				sinkstate = SEND_PACKET;
			}
		}
		break;
	case SEND_PACKET:
		if (!in[index].empty() && !out.full())
		{
			temp = in[index].read();

			if (total_flit_count % config_t::MAX_PACKET_LENGTH == config_t::MAX_PACKET_LENGTH-1)
			{
				temp.last = 1;
			}
			else if (total_flit_count == total_num_flit - 1)
			{
				temp.last = 1;
			}
			else
			{
				temp.last = 0;
			}

			temp.id = config_t::id;
			temp.dest = config_t::dest;
			temp.user = message_length;
			out.write(temp);

			if (flit_count == num_flit_per_child[index] - 1 && index == config_t::NUM_CHILDREN - 1)
			{
				total_flit_count = 0;
				flit_count = 0;
				index = 0;
				sinkstate = IDLE;
			}
			else if (flit_count == num_flit_per_child[index] - 1)
			{
				total_flit_count += 1;
				flit_count = 0;
				index += 1;
				sinkstate = SEND_PACKET_FIRST_FLIT;
			}
			else
			{
				total_flit_count += 1;
				flit_count += 1;
				sinkstate = SEND_PACKET;
			}
		}
		break;
	default:
		break;
	}
}

template<typename config_t>
void GatherRecvSpine(
		hls::stream<dataword>& in,
		hls::stream<dataword> out[config_t::NUM_FIFO])
{
#pragma HLS inline
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE axis port=in

	dataword temp;
	ap_uint<1> not_full = 1;
	for (int i=0; i<config_t::NUM_FIFO; i++) {
		#pragma HLS unroll
		not_full &= ~out[i].full();
	}

	if (!in.empty() && not_full)
	{
		temp = in.read();

		if (temp.id >= config_t::child_id_start && temp.id <= config_t::child_id_end)
		{
			out[temp.id - config_t::child_id_start].write(temp);
		}
		else
		{
			out[temp.id - config_t::node_id_start + config_t::NUM_CHILDREN].write(temp);
		}
	}
}

template<typename config_t>
void GatherSendSpine(
		hls::stream<dataword> in[config_t::NUM_FIFO],
		hls::stream<dataword>& out)
{
#pragma HLS inline
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE axis port=in

	enum state {IDLE=0, SEND_PACKET_FIRST_FLIT, SEND_PACKET};

	static dataword temp;
	static dataword first_flit_per_child[config_t::NUM_FIFO];
	static ap_uint<16> num_flit_per_child[config_t::NUM_FIFO]; 
	static ap_uint<16> total_num_flit;
	static state sinkstate = IDLE;
	static ap_uint<8> index;
	static ap_uint<16> flit_count;
	static ap_uint<16> total_flit_count;
	static ap_uint<16> message_length;

#pragma HLS array_partition variable=first_flit_per_child dim=0 complete
#pragma HLS array_partition variable=num_flit_per_child dim=0 complete

	switch(sinkstate)
	{
	case IDLE:
		{
			ap_uint<1> not_empty = 1;
			for (int i=0; i<config_t::NUM_FIFO; i++) {
				#pragma HLS unroll
				not_empty &= ~in[i].empty();
			}

			if (not_empty && !out.full())
			{
				for (int i=0; i<config_t::NUM_FIFO; i++) {
					#pragma HLS unroll
					first_flit_per_child[i] = in[i].read();
				}

				for (int i=0; i<config_t::NUM_FIFO; i++) {
					#pragma HLS unroll

					if (first_flit_per_child[i].user % PACKET_NUM_BYTES == 0)
					{
						num_flit_per_child[i] = first_flit_per_child[i].user / PACKET_NUM_BYTES;
					}
					else
					{
						num_flit_per_child[i] = first_flit_per_child[i].user / PACKET_NUM_BYTES + 1;
					}
				}

				total_num_flit = 0;
				for (int i=0; i<config_t::NUM_FIFO; i++) {
					#pragma HLS unroll
					total_num_flit += num_flit_per_child[i];
				}

				message_length = 0;
				for (int i=0; i<config_t::NUM_FIFO; i++) {
					#pragma HLS unroll
					message_length += first_flit_per_child[i].user;
				}
				
				index = 0;
				flit_count = 0;
				total_flit_count = 0;

				sinkstate = SEND_PACKET_FIRST_FLIT;
			}
		}
		break;
	case SEND_PACKET_FIRST_FLIT:
		if (!out.full())
		{
			temp.data = first_flit_per_child[index].data;

			if (total_flit_count % config_t::MAX_PACKET_LENGTH == config_t::MAX_PACKET_LENGTH-1)
			{
				temp.last = 1;
			}
			else if (total_flit_count == total_num_flit - 1)
			{
				temp.last = 1;
			}
			else
			{
				temp.last = 0;
			}

			temp.id = config_t::id;
			temp.dest = config_t::dest;
			temp.user = message_length;
			out.write(temp);

			if (flit_count == num_flit_per_child[index] - 1 && index == config_t::NUM_FIFO - 1)
			{
				total_flit_count = 0;
				flit_count = 0;
				index = 0;
				sinkstate = IDLE;
			}
			else if (flit_count == num_flit_per_child[index] - 1)
			{
				total_flit_count += 1;
				flit_count = 0;
				index += 1;
				sinkstate = SEND_PACKET_FIRST_FLIT;
			}
			else
			{
				total_flit_count += 1;
				flit_count += 1;
				sinkstate = SEND_PACKET;
			}
		}
		break;
	case SEND_PACKET:
		if (!in[index].empty() && !out.full())
		{
			temp = in[index].read();

			if (total_flit_count % config_t::MAX_PACKET_LENGTH == config_t::MAX_PACKET_LENGTH-1)
			{
				temp.last = 1;
			}
			else if (total_flit_count == total_num_flit - 1)
			{
				temp.last = 1;
			}
			else
			{
				temp.last = 0;
			}

			temp.id = config_t::id;
			temp.dest = config_t::dest;
			temp.user = message_length;
			out.write(temp);

			if (flit_count == num_flit_per_child[index] - 1 && index == config_t::NUM_FIFO - 1)
			{
				total_flit_count = 0;
				flit_count = 0;
				index = 0;
				sinkstate = IDLE;
			}
			else if (flit_count == num_flit_per_child[index] - 1)
			{
				total_flit_count += 1;
				flit_count = 0;
				index += 1;
				sinkstate = SEND_PACKET_FIRST_FLIT;
			}
			else
			{
				total_flit_count += 1;
				flit_count += 1;
				sinkstate = SEND_PACKET;
			}
		}
		break;
	default:
		break;
	}
}


template<typename config_t>
void ScatterLeaf(
		const ap_uint<PACKET_DEST_LENGTH> children_id[config_t::NUM_CHILDREN],
		hls::stream<dataword>& in,
		hls::stream<dataword>& out
		)
{
#pragma HLS array_partition variable=children_id dim=0 complete
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	enum state {IDLE=0, SEND_FIRST_FLIT_TO_FIRST_CHILD, SEND_TO_CHILDREN};

	static state sinkstate = IDLE;
	static dataword temp;
	static ap_uint<8> child_count;
	static ap_uint<16> flit_count;
	static ap_uint<8> node_count; 
	static ap_uint<16> message_length;
	static ap_uint<16> num_flit_per_child[config_t::NUM_CHILDREN]; 
	static ap_uint<16> num_message_per_child[config_t::NUM_CHILDREN]; 

#pragma HLS array_partition variable=num_flit_per_child dim=0 complete
#pragma HLS array_partition variable=num_message_per_child dim=0 complete


	switch(sinkstate)
	{
	case IDLE:
		if (!in.empty() && !out.full())
		{
			temp = in.read();

			message_length = temp.user;

			ap_uint<16> message_length_per_kern = message_length / config_t::TOTAL_NUM_CHILDREN;
			ap_uint<16> message_length_per_kern_last = message_length_per_kern + message_length % config_t::TOTAL_NUM_CHILDREN;
			ap_uint<16> total_num_flit;
			
			if (message_length % PACKET_NUM_BYTES == 0)
			{
				total_num_flit = message_length / PACKET_NUM_BYTES;
			}
			else
			{
				total_num_flit = message_length / PACKET_NUM_BYTES + 1;
			}

			ap_uint<16> temp_num_flit_per_child = total_num_flit / config_t::TOTAL_NUM_CHILDREN;
			ap_uint<16> temp_num_flit_per_child_last = total_num_flit / config_t::TOTAL_NUM_CHILDREN + total_num_flit % config_t::TOTAL_NUM_CHILDREN;

			for (int i=0; i<config_t::NUM_CHILDREN-1; i++ ) {
				#pragma HLS unroll
				num_message_per_child[i] = message_length_per_kern;
			}

			if (config_t::LAST_NODE == 1)
			{
				num_message_per_child[config_t::NUM_CHILDREN-1] = message_length_per_kern_last;
			}
			else
			{
				num_message_per_child[config_t::NUM_CHILDREN-1] = message_length_per_kern;
			}

			for (int i=0; i<config_t::NUM_CHILDREN-1; i++ ) {
				#pragma HLS unroll
				num_flit_per_child[i] = temp_num_flit_per_child;
			}

			if (config_t::LAST_NODE == 1)
			{
				num_flit_per_child[config_t::NUM_CHILDREN-1] = temp_num_flit_per_child_last;
			}
			else
			{
				num_flit_per_child[config_t::NUM_CHILDREN-1] = temp_num_flit_per_child;
			}
			
			
			flit_count = 0;
			child_count = 0;
			node_count = 0;

			sinkstate = SEND_FIRST_FLIT_TO_FIRST_CHILD;
		}
		break;
	case SEND_FIRST_FLIT_TO_FIRST_CHILD:
		if (!out.full())
		{
			temp.id = config_t::id;
			temp.dest = children_id[child_count];
			temp.user = num_message_per_child[child_count];
			
			if (flit_count % config_t::MAX_PACKET_LENGTH == config_t::MAX_PACKET_LENGTH-1)
			{
				temp.last = 1;
			}
			else
			{
				temp.last = flit_count == num_flit_per_child[child_count]-ap_uint<16>(1) ? 1:0;
			}

			out.write(temp);

			if (flit_count == num_flit_per_child[child_count] - 1 && child_count == config_t::NUM_CHILDREN - 1)
			{
				flit_count = 0;
				child_count = 0;
				sinkstate = IDLE;
			}
			else if (flit_count == num_flit_per_child[child_count] - 1)
			{
				flit_count = 0;
				child_count += 1;
				sinkstate = SEND_TO_CHILDREN;
			}
			else
			{
				flit_count += 1;
				sinkstate = SEND_TO_CHILDREN;
			}
		}
		break;
	case SEND_TO_CHILDREN:
		if (!in.empty() && !out.full())
		{
			temp = in.read();
			temp.id = config_t::id;
			temp.dest = children_id[child_count];
			temp.user = num_message_per_child[child_count];

			if (flit_count % config_t::MAX_PACKET_LENGTH == config_t::MAX_PACKET_LENGTH-1)
			{
				temp.last = 1;
			}
			else
			{
				temp.last = flit_count == num_flit_per_child[child_count]-ap_uint<16>(1) ? 1:0;
			}
			
			out.write(temp);

			if (flit_count == num_flit_per_child[child_count] - 1 && child_count == config_t::NUM_CHILDREN - 1)
			{
				flit_count = 0;
				child_count = 0;
				sinkstate = IDLE;
			}
			else if (flit_count == num_flit_per_child[child_count] - 1)
			{
				flit_count = 0;
				child_count += 1;
				sinkstate = SEND_TO_CHILDREN;
			}
			else
			{
				flit_count += 1;
				sinkstate = SEND_TO_CHILDREN;
			}
		}
		break;
	}
}


template<typename config_t>
void ScatterSpine(
		const ap_uint<PACKET_DEST_LENGTH> children_id[config_t::NUM_CHILDREN],
		const ap_uint<PACKET_DEST_LENGTH> node_id[config_t::NUM_NODE],
		const ap_uint<PACKET_DEST_LENGTH> num_children_per_node[config_t::NUM_NODE],
		hls::stream<dataword>& in,
		hls::stream<dataword>& out
		)
{
#pragma HLS array_partition variable=children_id dim=0 complete
#pragma HLS array_partition variable=node_id dim=0 complete
#pragma HLS array_partition variable=num_children_per_node dim=0 complete
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	enum state {IDLE=0, SEND_FIRST_FLIT_TO_FIRST_CHILD, SEND_TO_CHILDREN, SEND_TO_NODE};

	static state sinkstate = IDLE;
	static dataword temp;
	static ap_uint<8> child_count;
	static ap_uint<16> flit_count;
	static ap_uint<8> node_count; 
	static ap_uint<16> message_length;
	static ap_uint<16> num_flit_per_node[config_t::NUM_NODE]; 
	static ap_uint<16> num_flit_per_child[config_t::NUM_CHILDREN]; 
	static ap_uint<16> num_message_per_child[config_t::NUM_CHILDREN]; 

#pragma HLS array_partition variable=num_flit_per_node dim=0 complete
#pragma HLS array_partition variable=num_flit_per_child dim=0 complete
#pragma HLS array_partition variable=num_message_per_child dim=0 complete


	switch(sinkstate)
	{
	case IDLE:
		if (!in.empty() && !out.full())
		{
			temp = in.read();

			message_length = temp.user;

			ap_uint<16> message_length_per_kern = message_length / config_t::TOTAL_NUM_CHILDREN;
			ap_uint<16> message_length_per_kern_last = message_length_per_kern + message_length % config_t::TOTAL_NUM_CHILDREN;
			ap_uint<16> total_num_flit;
			
			if (message_length % PACKET_NUM_BYTES == 0)
			{
				total_num_flit = message_length / PACKET_NUM_BYTES;
			}
			else
			{
				total_num_flit = message_length / PACKET_NUM_BYTES + 1;
			}

			ap_uint<16> temp_num_flit_per_child = total_num_flit / config_t::TOTAL_NUM_CHILDREN;
			ap_uint<16> temp_num_flit_per_child_last = total_num_flit / config_t::TOTAL_NUM_CHILDREN + total_num_flit % config_t::TOTAL_NUM_CHILDREN;

			for (int i=0; i<config_t::NUM_CHILDREN; i++ ) {
				#pragma HLS unroll
				num_message_per_child[i] = message_length_per_kern;
			}

			for (int i=0; i<config_t::NUM_CHILDREN; i++ ) {
				#pragma HLS unroll
				num_flit_per_child[i] = temp_num_flit_per_child;
			}
			
			for (int i=0; i<config_t::NUM_NODE-1; i++ ) {
				#pragma HLS unroll
				num_flit_per_node[i] = temp_num_flit_per_child * num_children_per_node[i];
			}

			num_flit_per_node[config_t::NUM_NODE-1] = temp_num_flit_per_child * (num_children_per_node[config_t::NUM_NODE-1]-1) + temp_num_flit_per_child_last;

			flit_count = 0;
			child_count = 0;
			node_count = 0;

			sinkstate = SEND_FIRST_FLIT_TO_FIRST_CHILD;
		}
		break;
	case SEND_FIRST_FLIT_TO_FIRST_CHILD:
		if (!out.full())
		{
			temp.id = config_t::id;
			temp.dest = children_id[child_count];
			temp.user = num_message_per_child[child_count];
			
			if (flit_count % config_t::MAX_PACKET_LENGTH == config_t::MAX_PACKET_LENGTH-1)
			{
				temp.last = 1;
			}
			else
			{
				temp.last = flit_count == num_flit_per_child[child_count]-ap_uint<16>(1) ? 1:0;
			}

			out.write(temp);

			if (flit_count == num_flit_per_child[child_count] - 1 && child_count == config_t::NUM_CHILDREN - 1)
			{
				flit_count = 0;
				child_count = 0;
				sinkstate = SEND_TO_NODE;
			}
			else if (flit_count == num_flit_per_child[child_count] - 1)
			{
				flit_count = 0;
				child_count += 1;
				sinkstate = SEND_TO_CHILDREN;
			}
			else
			{
				flit_count += 1;
				sinkstate = SEND_TO_CHILDREN;
			}
		}
		break;
	case SEND_TO_CHILDREN:
		if (!in.empty() && !out.full())
		{
			temp = in.read();
			temp.id = config_t::id;
			temp.dest = children_id[child_count];
			temp.user = num_message_per_child[child_count];

			if (flit_count % config_t::MAX_PACKET_LENGTH == config_t::MAX_PACKET_LENGTH-1)
			{
				temp.last = 1;
			}
			else
			{
				temp.last = flit_count == num_flit_per_child[child_count]-ap_uint<16>(1) ? 1:0;
			}
			
			out.write(temp);

			if (flit_count == num_flit_per_child[child_count] - 1 && child_count == config_t::NUM_CHILDREN - 1)
			{
				flit_count = 0;
				child_count = 0;
				sinkstate = SEND_TO_NODE;
			}
			else if (flit_count == num_flit_per_child[child_count] - 1)
			{
				flit_count = 0;
				child_count += 1;
				sinkstate = SEND_TO_CHILDREN;
			}
			else
			{
				flit_count += 1;
				sinkstate = SEND_TO_CHILDREN;
			}
		}
		break;
	case SEND_TO_NODE:
		if (!in.empty() && !out.full())
		{
			temp = in.read();
			temp.id = config_t::id;
			temp.dest = node_id[node_count];
			temp.user = message_length;

			if (flit_count % config_t::MAX_PACKET_LENGTH == config_t::MAX_PACKET_LENGTH-1)
			{
				temp.last = 1;
			}
			else
			{
				temp.last = flit_count == num_flit_per_node[node_count]-ap_uint<16>(1) ? 1:0;
			}
			
			out.write(temp);

			if (flit_count == num_flit_per_node[node_count]-1 && node_count == config_t::NUM_NODE-1)
			{
				flit_count = 0;
				node_count = 0;
				sinkstate = IDLE;
			}
			else if (flit_count == num_flit_per_node[node_count]-1)
			{
				flit_count = 0;
				node_count += 1;
				sinkstate = SEND_TO_NODE;
			}
			else
			{
				flit_count += 1;
				sinkstate = SEND_TO_NODE;
			}
		}
		break;
	}
}


template<typename config_t>
void Bcast(
		const ap_uint<PACKET_DEST_LENGTH> children_id[config_t::NUM_CHILDREN],
		hls::stream<dataword>& in,
		hls::stream<dataword>& out,
		hls::stream<dataword>& in_fifo,
		hls::stream<dataword>& out_fifo
		)
{
#pragma HLS inline
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE axis port=in_fifo
#pragma HLS INTERFACE axis port=out_fifo

#pragma HLS array_partition variable=children_id dim=0 complete


	enum state {IDLE=0, SEND_PACKET_TO_LOCAL, SEND_PACKET_TO_LOCAL_AND_ONE_CHILD, SEND_PACKET_TO_CHILDREN};

	dataword pkt_in;
	static state sinkstate=IDLE;
	static ap_uint<8> dest_count;

	switch (sinkstate)
	{
	case IDLE:
		if (!in.empty() && !out.full() && !out_fifo.full())
		{
			pkt_in = in.read();
			dest_count = 0;
			pkt_in.id = config_t::id;
			pkt_in.dest = children_id[dest_count];
			out.write(pkt_in);

			if (dest_count < config_t::NUM_CHILDREN - 1)
			{
				out_fifo.write(pkt_in);
			}

			if (pkt_in.last)
			{
				if (dest_count == config_t::NUM_CHILDREN - 1)
				{
					sinkstate = IDLE;
				}
				else
				{
					dest_count += 1;
					sinkstate = SEND_PACKET_TO_CHILDREN;
				}
			}
			else
			{
				sinkstate = SEND_PACKET_TO_LOCAL_AND_ONE_CHILD;
			}
		}
		break;
	case SEND_PACKET_TO_LOCAL_AND_ONE_CHILD:
		if (!in.empty() && !out.full() && !out_fifo.full())
		{
			pkt_in = in.read();
			pkt_in.id = config_t::id;
			pkt_in.dest = children_id[dest_count];
			out.write(pkt_in);

			if (dest_count < config_t::NUM_CHILDREN - 1)
			{
				out_fifo.write(pkt_in);
			}

			if (pkt_in.last)
			{
				if (dest_count == config_t::NUM_CHILDREN - 1)
				{
					sinkstate = IDLE;
				}
				else
				{
					dest_count += 1;
					sinkstate = SEND_PACKET_TO_CHILDREN;
				}
			}
			else
			{
				sinkstate = SEND_PACKET_TO_LOCAL_AND_ONE_CHILD;
			}
		}
		break;
	case SEND_PACKET_TO_CHILDREN:
		if (!in_fifo.empty() && !out.full() && !out_fifo.full())
		{
			pkt_in = in_fifo.read();
			pkt_in.id = config_t::id;
			pkt_in.dest = children_id[dest_count];
			out.write(pkt_in);

			if (dest_count < config_t::NUM_CHILDREN - 1)
			{
				out_fifo.write(pkt_in);
			}

			if (pkt_in.last)
			{
				if (dest_count == config_t::NUM_CHILDREN - 1)
				{
					sinkstate = IDLE;
				}
				else
				{
					dest_count += 1;
					sinkstate = SEND_PACKET_TO_CHILDREN;
				}
			}
			else
			{
				sinkstate = SEND_PACKET_TO_CHILDREN;
			}
		}
		break;
	default:
		break;
	}
}



template<typename config_t>
void ReduceRecvLeaf(
		hls::stream<dataword>& in,
		hls::stream<dataword> out[config_t::NUM_CHILDREN])
{
#pragma HLS inline
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE axis port=in

	dataword temp;
	ap_uint<1> not_full = 1;
	for (int i=0; i<config_t::NUM_CHILDREN; i++) {
		#pragma HLS unroll
		not_full &= ~out[i].full();
	}

	if (!in.empty() && not_full)
	{
		temp = in.read();
		out[temp.id - config_t::base_src].write(temp);
	}
}

template<typename config_t>
void ReduceRecvSpine(
		hls::stream<dataword>& in,
		hls::stream<dataword> out[config_t::NUM_FIFO])
{
#pragma HLS inline
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE axis port=in

	dataword temp;
	ap_uint<1> not_full = 1;
	for (int i=0; i<config_t::NUM_FIFO; i++) {
		#pragma HLS unroll
		not_full &= ~out[i].full();
	}

	if (!in.empty() && not_full)
	{
		temp = in.read();

		if (temp.id >= config_t::child_id_start && temp.id <= config_t::child_id_end)
		{
			out[temp.id - config_t::child_id_start].write(temp);
		}
		else
		{
			out[temp.id - config_t::node_id_start + config_t::NUM_CHILDREN].write(temp);
		}
	}
}

template<typename config_t>
void ReduceSumLeaf(
		hls::stream<dataword> in[config_t::NUM_CHILDREN],
		hls::stream<dataword>& out)
{
#pragma HLS inline
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE axis port=in

	static dataword temp;

	ap_uint<8> sum_val[64];
	static dataword flit_per_child[config_t::NUM_CHILDREN];

#pragma HLS array_partition variable=flit_per_child dim=0 complete
#pragma HLS array_partition variable=sum_val dim=0 complete

	ap_uint<1> not_empty = 1;
	for (int i=0; i<config_t::NUM_CHILDREN; i++) {
		#pragma HLS unroll
		not_empty &= ~in[i].empty();
	}


	if (not_empty && !out.full())
	{
		for (int i=0; i<config_t::NUM_CHILDREN; i++) {
			#pragma HLS unroll
			flit_per_child[i] = in[i].read();
		}

		for (int j=0; j<64; j++) {
			#pragma HLS unroll
			sum_val[j] = 0;
		}
		
		for (int j=0; j<64; j++) {
			#pragma HLS unroll
			for (int i=0; i<config_t::NUM_CHILDREN; i++) {
				#pragma HLS unroll
				sum_val[j] += flit_per_child[i].data.range(j*8+7, j*8);
			}
		}

		for (int j=0; j<64; j++) {
			#pragma HLS unroll
			temp.data.range(j*8+7, j*8) = sum_val[j];
		}

		temp.id = config_t::id;
		temp.dest = config_t::dest;
		temp.user = flit_per_child[0].user;
		temp.last = flit_per_child[0].last;
		out.write(temp);
	}
}


template<typename config_t>
void ReduceSumSpine(
		hls::stream<dataword> in[config_t::NUM_FIFO],
		hls::stream<dataword>& out)
{
#pragma HLS inline
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE axis port=in

	static dataword temp;

	ap_uint<8> sum_val[64];
	static dataword flit_per_child[config_t::NUM_FIFO];

#pragma HLS array_partition variable=flit_per_child dim=0 complete
#pragma HLS array_partition variable=sum_val dim=0 complete

	ap_uint<1> not_empty = 1;
	for (int i=0; i<config_t::NUM_FIFO; i++) {
		#pragma HLS unroll
		not_empty &= ~in[i].empty();
	}


	if (not_empty && !out.full())
	{
		for (int i=0; i<config_t::NUM_FIFO; i++) {
			#pragma HLS unroll
			flit_per_child[i] = in[i].read();
		}

		for (int j=0; j<64; j++) {
			#pragma HLS unroll
			sum_val[j] = 0;
		}
		
		for (int j=0; j<64; j++) {
			#pragma HLS unroll
			for (int i=0; i<config_t::NUM_FIFO; i++) {
				#pragma HLS unroll
				sum_val[j] += flit_per_child[i].data.range(j*8+7, j*8);
			}
		}

		for (int j=0; j<64; j++) {
			#pragma HLS unroll
			temp.data.range(j*8+7, j*8) = sum_val[j];
		}

		temp.id = config_t::id;
		temp.dest = config_t::dest;
		temp.user = flit_per_child[0].user;
		temp.last = flit_per_child[0].last;
		out.write(temp);
	}
}
