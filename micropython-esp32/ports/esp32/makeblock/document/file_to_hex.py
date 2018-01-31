OUT_FILE_NAME = "hex_out.txt"
READ_FILE_NAME = "main.py"
FILE_NAME = "main.py"
FILE_BLOCK_SIZE = 20

FTP_HEADER = bytes.fromhex("F3 00 5E 01")
FTP_END = bytes.fromhex("F3 00 5E 01")
frame_header_str = "F3"
frame_end_str = "F4"
dev_id_str = "00"
srv_id_str = "5E"
file_header_cmd_id_str = "01"
file_block_cmd_id_str = "02"
file_state_cmd_id_str = "F0"
file_type_str = "00"

FRAME_HEAD = 0xF3
FRAME_END = 0xF4
DEV_ID = 0x00
SRV_ID = 0x5E
CMD_STATE_ID = 0xF0

def print_bytes_hex( bytes_data ):
	print( ":".join( "{:02x}".format(c) for c in bytes_data ) )

def bytes_to_hex_str( bytes_data ):
	return " ".join( "{:02x}".format(c) for c in bytes_data )

def calc_add_checksum( data ):
	ret = 0
	for c in data:
		ret = ret + c
	return ret & 0xFF

def calc_32bit_xor( data ):
	bytes_len = len( data )
	data_bytes = bytes( data, encoding = 'utf-8' )
	# print_bytes_hex( data_bytes )
	# print( bytes_len/4 )
	# print( int(bytes_len/4) )
	checksum = bytearray.fromhex( "00 00 00 00" )
	for i in range(int(bytes_len/4)):
		checksum[0] = checksum[0] ^ data_bytes[i*4 + 0]
		checksum[1] = checksum[1] ^ data_bytes[i*4 + 1]
		checksum[2] = checksum[2] ^ data_bytes[i*4 + 2]
		checksum[3] = checksum[3] ^ data_bytes[i*4 + 3]

	if ( bytes_len%4 ):
		for i in range( bytes_len%4 ):
			checksum[0+i] = checksum[0+i] ^ data_bytes[4*int(bytes_len/4) + i]

	# print_bytes_hex( checksum )
	return checksum

def out_file( in_file_name, out_file_name ):
	# read file 
	in_f = open( in_file_name, 'r' )
	in_f_d = in_f.read()
	in_f_len = len(in_f_d)

	# out file
	out_f = open( out_file_name, 'w' )
	
	# send file header
	cmd_len_str = bytes_to_hex_str( (0x09 + len(FILE_NAME)).to_bytes( 1, byteorder='little' ) )
	file_size_str = bytes_to_hex_str( in_f_len.to_bytes(4, byteorder='little') )
	file_checksum_str = bytes_to_hex_str( calc_32bit_xor( in_f_d ) )
	file_name_str = bytes_to_hex_str( bytes( FILE_NAME, encoding = 'utf-8' ) )
	frame_data_str = dev_id_str + " " + srv_id_str + " " + file_header_cmd_id_str + " " + cmd_len_str + " " + file_type_str + " " + file_size_str + " " + file_checksum_str + " " + file_name_str;
	frame_checksum_str = bytes_to_hex_str( calc_add_checksum( bytes.fromhex( frame_data_str ) ).to_bytes(1, byteorder='little' ) )
	send_head_str = frame_header_str + " " + frame_data_str + " " + frame_checksum_str + " " + frame_end_str
	print( send_head_str )
	out_f.write( "# FILE HEADER\r\n" )
	out_f.write( send_head_str )

	# send file block
	file_offset = 0
	i = 0
	while ( file_offset < in_f_len ):
		print( "==== %% %f"%(100*file_offset/in_f_len) )
		if ( file_offset + FILE_BLOCK_SIZE <  in_f_len ):
			send_file_size = FILE_BLOCK_SIZE
		else:
			send_file_size = in_f_len - file_offset

		file_offset_str = bytes_to_hex_str( file_offset.to_bytes( 4, byteorder='little' ) )
		cmd_len_str = bytes_to_hex_str( (0x04 + send_file_size).to_bytes( 1, byteorder='little' ) )
		file_block_str = bytes_to_hex_str( bytes( in_f_d[file_offset:file_offset+send_file_size], encoding='utf-8' ) )
		frame_data_str = dev_id_str + " " + srv_id_str + " " + file_block_cmd_id_str + " " + cmd_len_str + " " + file_offset_str + " " + file_block_str;
		frame_checksum_str = bytes_to_hex_str( calc_add_checksum( bytes.fromhex( frame_data_str ) ).to_bytes(1, byteorder='little' ) )
		send_block_str = frame_header_str + " " + frame_data_str + " " + frame_checksum_str + " " + frame_end_str
		print( send_block_str )
		out_f.write( "\r\nBLOCK #%d\r\n"%i )
		out_f.write( send_block_str )
		file_offset = file_offset + send_file_size
		i = i+1

	in_f.close()
	out_f.close()

out_file( READ_FILE_NAME, OUT_FILE_NAME  )
input()