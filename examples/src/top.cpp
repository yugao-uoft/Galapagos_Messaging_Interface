#include "common.hpp" 
#include "modules.hpp" 
#include "parameters.hpp" 


void test_sender(
	hls::stream<dataword>& in,
	hls::stream<dataword>& out,
	hls::stream<dataword> out_fifo)
{
#pragma HLS interface ap_ctrl_none port=return 	
#pragma HLS INTERFACE axis port=out_fifo
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out
	
	enum state {IDLE=0, SEND_PACKET};

	static enum state sinkstate = IDLE;
	static int message_length;
	static int num_flit;
	static int flit_count;
	static dataword temp;
	static ap_uint<8> id;
	static ap_uint<8> dest;

	switch(sinkstate)
	{
	case IDLE:
		if (!in.empty() && !out.full() && !out_fifo.full())
		{
			temp = in.read();
			message_length = temp.data.range(31,0); 
			id = temp.dest;
			dest = temp.data.range(63,32);

			if (message_length < PACKET_NUM_BYTES) 
			{
				num_flit = 1;
			} 
			else if (message_length % PACKET_NUM_BYTES == 0) 
			{
				num_flit = message_length / PACKET_NUM_BYTES;
			} 
			else 
			{
				num_flit = message_length / PACKET_NUM_BYTES + 1;
			}

			dataword metadata;
			metadata.data(31,0) = message_length;
			out_fifo.write(metadata);

			sinkstate = SEND_PACKET;
		}
		break;
	case SEND_PACKET:
		if (!out.full())
		{
			for (int k=0; k<64;k++) {
				#pragma HLS UNROLL
				temp.data(k * 8 + 7, k * 8) = k;
			}

			temp.id = id;
			temp.dest = dest;
			temp.user = message_length;
			temp.last = flit_count == num_flit-ap_uint<16>(1) ? 1:0;
			out.write(temp);

			if (flit_count == num_flit-1)
			{
				flit_count = 0;
				sinkstate = IDLE;
			}
			else
			{
				flit_count += 1;
				sinkstate = SEND_PACKET;
			}
		}
		break;
	default:
		break;
	}
}

void test_receiver(
	hls::stream<dataword>& in,
	hls::stream<dataword>& in_sender,
	volatile ap_uint<32> * counter_out)
{
#pragma HLS interface ap_ctrl_none port=return 	
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=in_sender
	
	enum state {IDLE=0, RECV_PACKET};

	static enum state sinkstate = IDLE;
	static int N;
	static int M;
	static int num_flit;
	static int flit_count;
	static int n_count;
	static int counter;
	dataword temp;
	static dataword out_data;
	static ap_uint<8> cpu_id;
	static ap_uint<8> id;
	static int message_length;

	*counter_out = counter;

	switch(sinkstate)
	{
	case IDLE:
		if (!in_sender.empty())
		{
			dataword metadata = in_sender.read();
			message_length = metadata.data(31,0) / config_t_test_receiver::num_scatter;
			
			if (message_length < PACKET_NUM_BYTES) 
			{
				num_flit = 1 * config_t_test_receiver::num_src;
			} 
			else if (message_length % PACKET_NUM_BYTES == 0) 
			{
				num_flit = message_length / PACKET_NUM_BYTES * config_t_test_receiver::num_src;
			} 
			else 
			{
				num_flit = (message_length / PACKET_NUM_BYTES + 1)  * config_t_test_receiver::num_src;
			}

			flit_count = 0;
			counter = 0;
			sinkstate = RECV_PACKET;
		}
		break;
	case RECV_PACKET:
		{
			if (!in.empty())
			{
				temp = in.read();
				
				if (flit_count == num_flit - 1)
				{
					sinkstate = IDLE;
				}
				else
				{
					flit_count ++;
					sinkstate = RECV_PACKET;
				}
			}

			counter += 1;
		}
		break;
	default:
		break;
	}
}

void packetizer(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out,
		volatile ap_uint<8> * out_state,
		volatile ap_uint<8> * out_num_packet,
		volatile ap_uint<8> * out_packet_length,
		volatile ap_uint<8> * out_packet_length_last) 
{
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

enum state {IDLE=0, SEND_PACKET_FIRST_FLIT, SEND_PACKET, SEND_PACKET_LAST};

	static state sinkstate = IDLE;
	static dataword temp;
	static ap_uint<8> packet_length;
	static ap_uint<8> packet_length_last;
	static ap_uint<8> num_packet;
	static ap_uint<8> packet_count;
	static ap_uint<8> flit_count;
	static ap_uint<16> num_flit;
	static int message_length;

	*out_state = sinkstate;
	*out_num_packet = num_packet;
	*out_packet_length = packet_length;
	*out_packet_length_last = packet_length_last;



	switch(sinkstate)
	{
	case IDLE:
		if (!in.empty() && !out.full())
		{
			temp = in.read();

			message_length = temp.user;

			if (message_length < PACKET_NUM_BYTES) 
			{
				num_flit = 1;
			} 
			else if (message_length % PACKET_NUM_BYTES == 0) 
			{
				num_flit = message_length / PACKET_NUM_BYTES;
			} 
			else 
			{
				num_flit = message_length / PACKET_NUM_BYTES + 1;
			}

			if (num_flit < 16)
			{
				packet_length = num_flit;
				packet_length_last = num_flit;
				num_packet = 1;
			}
			else if (num_flit % 16 == 0)
			{
				packet_length = 16;
				packet_length_last = 16;
				num_packet = num_flit / 16;
			}
			else 
			{
				packet_length = 16;
				packet_length_last = num_flit % 16;
				num_packet = num_flit / 16 + 1;
			}
			
			
			packet_count = 0;
			flit_count = 0;

			sinkstate = SEND_PACKET_FIRST_FLIT;
		}
		break;
	case SEND_PACKET_FIRST_FLIT:
		if (!out.full())
		{
			ap_uint<1> last;

			if (num_packet >= 2)
			{
				if (flit_count == packet_length-1 && packet_count == num_packet-2)
				{
					last = 1;
					flit_count = 0;
					packet_count = 0;
					sinkstate = SEND_PACKET_LAST;
				}
				else if (flit_count == packet_length - 1)
				{
					last = 1;
					flit_count = 0;
					packet_count += 1;
					sinkstate = SEND_PACKET;
				}
				else
				{
					last = 0;
					flit_count += 1;
					sinkstate = SEND_PACKET;
				}
			}
			else
			{
				if (flit_count == packet_length - 1)
				{
					last = 1;
					flit_count = 0;
					sinkstate = IDLE;
				}
				else
				{
					last = 0;
					flit_count += 1;
					sinkstate = SEND_PACKET;
				}
			}

			temp.last = last;
			out.write(temp);
		}
		break;
	case SEND_PACKET:
		if (!in.empty() && !out.full())
		{
			temp = in.read();

			ap_uint<1> last;

			if (num_packet >= 2)
			{
				if (flit_count == packet_length-1 && packet_count == num_packet-2)
				{
					last = 1;
					flit_count = 0;
					packet_count = 0;
					sinkstate = SEND_PACKET_LAST;
				}
				else if (flit_count == packet_length - 1)
				{
					last = 1;
					flit_count = 0;
					packet_count += 1;
					sinkstate = SEND_PACKET;
				}
				else
				{
					last = 0;
					flit_count += 1;
					sinkstate = SEND_PACKET;
				}
			}
			else
			{
				if (flit_count == packet_length - 1)
				{
					last = 1;
					flit_count = 0;
					sinkstate = IDLE;
				}
				else
				{
					last = 0;
					flit_count += 1;
					sinkstate = SEND_PACKET;
				}
			}

			temp.last = last;
			out.write(temp);
		}
		break;	
	case SEND_PACKET_LAST:
		if (!in.empty() && !out.full())
		{
			temp = in.read();

			ap_uint<1> last;

			if (flit_count == packet_length_last - 1)
			{
				last = 1;
				flit_count = 0;
				packet_count = 0;
				sinkstate = IDLE;
			}
			else
			{
				last = 0;
				flit_count += 1;
				sinkstate = SEND_PACKET_LAST;
			}

			temp.last = last;
			out.write(temp);
		}
		break;	

	}
}

void compute_4(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_4::id;
		temp.dest = config_t_compute_4::dest;
		out.write(temp);
	}
}
void compute_5(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_5::id;
		temp.dest = config_t_compute_5::dest;
		out.write(temp);
	}
}
void compute_6(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_6::id;
		temp.dest = config_t_compute_6::dest;
		out.write(temp);
	}
}
void compute_7(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_7::id;
		temp.dest = config_t_compute_7::dest;
		out.write(temp);
	}
}
void compute_8(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_8::id;
		temp.dest = config_t_compute_8::dest;
		out.write(temp);
	}
}
void compute_9(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_9::id;
		temp.dest = config_t_compute_9::dest;
		out.write(temp);
	}
}
void compute_10(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_10::id;
		temp.dest = config_t_compute_10::dest;
		out.write(temp);
	}
}
void compute_11(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_11::id;
		temp.dest = config_t_compute_11::dest;
		out.write(temp);
	}
}
void compute_12(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_12::id;
		temp.dest = config_t_compute_12::dest;
		out.write(temp);
	}
}
void compute_13(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_13::id;
		temp.dest = config_t_compute_13::dest;
		out.write(temp);
	}
}
void compute_14(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_14::id;
		temp.dest = config_t_compute_14::dest;
		out.write(temp);
	}
}
void compute_15(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_15::id;
		temp.dest = config_t_compute_15::dest;
		out.write(temp);
	}
}
void compute_16(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_16::id;
		temp.dest = config_t_compute_16::dest;
		out.write(temp);
	}
}
void compute_17(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_17::id;
		temp.dest = config_t_compute_17::dest;
		out.write(temp);
	}
}
void compute_18(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_18::id;
		temp.dest = config_t_compute_18::dest;
		out.write(temp);
	}
}
void compute_19(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

	if (!in.empty() && !out.full())
	{
		dataword temp = in.read();
		temp.id = config_t_compute_19::id;
		temp.dest = config_t_compute_19::dest;
		out.write(temp);
	}
}

void gather_recv_20(
		hls::stream<dataword>& in, 
		hls::stream<dataword> out[config_t_gather_20::NUM_CHILDREN]) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

GatherRecvLeaf<config_t_gather_20>(in, out); 
} 
void gather_send_20(
		hls::stream<dataword> in[config_t_gather_20::NUM_CHILDREN], 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

GatherSendLeaf<config_t_gather_20>(in, out); 
} 

void gather_recv_21(
		hls::stream<dataword>& in, 
		hls::stream<dataword> out[config_t_gather_21::NUM_CHILDREN]) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

GatherRecvLeaf<config_t_gather_21>(in, out); 
} 
void gather_send_21(
		hls::stream<dataword> in[config_t_gather_21::NUM_CHILDREN], 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

GatherSendLeaf<config_t_gather_21>(in, out); 
} 

void gather_recv_22(
		hls::stream<dataword>& in, 
		hls::stream<dataword> out[config_t_gather_22::NUM_FIFO]) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

GatherRecvSpine<config_t_gather_22>(in, out); 
} 
void gather_send_22(
		hls::stream<dataword> in[config_t_gather_22::NUM_FIFO], 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

GatherSendSpine<config_t_gather_22>(in, out); 
} 



void scatter_1(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

ScatterSpine<config_t_scatter_1>(scatter_1_children_id, scatter_1_node_id, scatter_1_num_children_per_node, in, out); 
} 


void scatter_2(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

ScatterLeaf<config_t_scatter_2>(scatter_2_children_id, in, out); 
} 

void scatter_3(
		hls::stream<dataword>& in, 
		hls::stream<dataword>& out) 
{ 
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out

ScatterLeaf<config_t_scatter_3>(scatter_3_children_id, in, out); 
} 
