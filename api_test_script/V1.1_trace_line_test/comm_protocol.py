import threading
import time
import sys
import serial
from threading import Timer
import random
import os

COM_PORT = "COM30"
READ_FILE_NAME = "mb_trace_line.py"
FTP_HEADER = bytes.fromhex("F3 00 5E 01")
FTP_END = bytes.fromhex("F3 00 5E 01")

STORE_FILE_NAME = "/flash/main.py"
# STORE_FILE_NAME = "/music/Superm.wav"
# STORE_FILE_NAME = "/flash/main.py"
# READ_FILE_NAME = "modnetwork.c"
# READ_FILE_NAME = "main.c"
# READ_FILE_NAME = "32bit_crc_test.txt"
# READ_FILE_NAME = "mb_factory_V11.py"
FILE_BLOCK_SIZE = 200

frame_header_str = "F3"
frame_end_str = "F4"
protocol_id_str = "01"
dev_id_str = "00"
srv_id_str = "5E"
file_header_cmd_id_str = "01"
file_block_cmd_id_str = "02"
file_state_cmd_id_str = "F0"
file_type_str = "88"

FRAME_HEAD = 0xF3
FRAME_END = 0xF4
DEV_ID = 0x00
SRV_ID = 0x5E
CMD_STATE_ID = 0xF0

FTP_FSM_HEAD_S = 0
FTP_FSM_HEAD_CHECK_S = 1
FTP_FSM_LEN1_S = 2
FTP_FSM_LEN2_S = 3
FTP_FSM_DATA_S = 4
FTP_FSM_CHECK_S = 5
FTP_FSM_END_S = 6

condition = threading.Condition()

# ------------------------ start ----------------------------------------------------------
ser = serial.Serial( COM_PORT, 115200 )

# These function input a int variable and return if it can be display
def is_display( c ):
	if ( 0x20 <= c and c <= 0x7E ):
		return True
	else:
		return False

def print_bytes_hex( bytes_data ):
	print( ":".join( "{:02x}".format(c) for c in bytes_data ) )

def bytes_to_hex_str( bytes_data ):
	return " ".join( "{:02x}".format(c) for c in bytes_data )

# data is bytesarray
# return is int
def calc_xor( data ):
	ret = 0
	for c in data:
		ret = ret ^ c
	return ret

def calc_add_checksum( data ):
	ret = 0
	for c in data:
		ret = ret + c
	return ret & 0xFF

def calc_32bit_xor( data ):
	bytes_len = len( data )
	# data_bytes = bytes( data, encoding = 'utf-8' )
	data_bytes = bytes( data )
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

	print_bytes_hex( checksum )
	return checksum

def get_file_len( file_name ):
	# read file 
	# f = open( file_name, 'r' )
	# f_d = f.read()
	# f_len = len(f_d)	
	# f.close()
	return os.path.getsize( "./file_name" )

def send_task():
	while( True ):
		#ser.write( "abcd".encode('utf-8') )
		#ser.write( b'\xF3\x00\x01\x02\x03\xff\xab' )
		#ser.write( bytes.fromhex("F3 00 5E 01") )
		#print( "abcd".encode('utf-8') )
		#ser.write( "abcd".encode('utf-8') )
		send_file( ser, READ_FILE_NAME, 0 )
		# end_time = time.time()
		# print( "Total time : %d second"%( end_time - start_time ), "avg tx speed: %d"% ( get_file_len( READ_FILE_NAME ) ) )
		# time.sleep(5)
		break

def send_file( ser, file_name, file_type ):
	# read file 
	f = open( file_name, 'rb' )
	f_d = f.read()
	f_len = len(f_d)

	# send file header
	while( True ):
		cmd_len_str = bytes_to_hex_str( (0x09 + len(STORE_FILE_NAME)).to_bytes( 2, byteorder='little' ) )
		file_size_str = bytes_to_hex_str( f_len.to_bytes(4, byteorder='little') )
		file_checksum_str = bytes_to_hex_str( calc_32bit_xor( f_d ) )
		file_name_str = bytes_to_hex_str( bytes( STORE_FILE_NAME, encoding = 'utf-8' ) )
		frame_data_str = protocol_id_str + " " + dev_id_str + " " + srv_id_str + " " + file_header_cmd_id_str + " " + cmd_len_str + " " + file_type_str + " " + file_size_str + " " + file_checksum_str + " " + file_name_str;
		frame_data_len = len( bytes.fromhex(frame_data_str) )
		frame_data_len_str = bytes_to_hex_str( (frame_data_len).to_bytes( 2, byteorder='little' ) )
		frame_head_checkusum_str = bytes_to_hex_str( calc_add_checksum( bytes.fromhex( frame_header_str+frame_data_len_str ) ).to_bytes(1, byteorder='little' ) )
		frame_checksum_str = bytes_to_hex_str( calc_add_checksum( bytes.fromhex( frame_data_str ) ).to_bytes(1, byteorder='little' ) )
		
		send_head_str = frame_header_str + " " + frame_head_checkusum_str + " " + frame_data_len_str + " " + frame_data_str + " " + frame_checksum_str + " " + frame_end_str
		print( send_head_str )
		
		ser.write( bytes.fromhex( send_head_str) )
		condition.acquire()
		if ( condition.wait( 5 ) ):
			print( "send file header ok" )
			break;
		else:
			print( "send file header err" )

	# wait for respond

	# send file block
	start_time = time.time()
	file_offset = 0
	while ( file_offset < f_len ):
		print( "==== %% %f"%(100*file_offset/f_len) )
		if ( (file_offset + FILE_BLOCK_SIZE) <  f_len ):
			send_file_size = FILE_BLOCK_SIZE
		else:
			send_file_size = f_len - file_offset

		file_offset_str = bytes_to_hex_str( file_offset.to_bytes( 4, byteorder='little' ) )
		cmd_len_str = bytes_to_hex_str( (0x04 + send_file_size).to_bytes( 2, byteorder='little' ) )
		# file_block_str = bytes_to_hex_str( bytes( f_d[file_offset:file_offset+send_file_size], encoding='utf-8' ) )
		file_block_str = bytes_to_hex_str( bytes( f_d[file_offset:file_offset+send_file_size] ) )
		frame_data_str = protocol_id_str + " " + dev_id_str + " " + srv_id_str + " " + file_block_cmd_id_str + " " + cmd_len_str + " " + file_offset_str + " " + file_block_str;
		frame_data_len = len( bytes.fromhex(frame_data_str) )
		frame_data_len_str = bytes_to_hex_str( (frame_data_len).to_bytes( 2, byteorder='little' ) )
		frame_head_checkusum_str = bytes_to_hex_str( calc_add_checksum( bytes.fromhex( frame_header_str+frame_data_len_str ) ).to_bytes(1, byteorder='little' ) )
		frame_checksum_str = bytes_to_hex_str( calc_add_checksum( bytes.fromhex( frame_data_str ) ).to_bytes(1, byteorder='little' ) )

		send_block_str = frame_header_str + " " + frame_head_checkusum_str + " " + frame_data_len_str + " " + frame_data_str + " " + frame_checksum_str + " " + frame_end_str
		# print( send_block_str )
		# random product err
		send_block_bytes = bytearray.fromhex( send_block_str);
		
		
		#if ( 5 == random.randint( 1, 10 ) ):
		#	print( "---->ERR send" )
		#	send_block_bytes[ random.randint(0, bytes.fromhex(cmd_len_str)[0] + 6) ] += 1

		ser.write( send_block_bytes )

		condition.acquire()
		if ( condition.wait( 1 ) ):
			file_offset = file_offset + send_file_size
		else:
			print( "&&&&&&&&&&& Resend a block" )
			time.sleep( 5 )
			continue
			

	print( "===============================================================================================" )
	print( ">>>>>> Spend time : %d second"%( time.time() - start_time ), "avg tx speed: %d"% (file_offset/(time.time() - start_time)) )
	print( ">>>>>> Total cnt %d"%f_len )
	print( "===============================================================================================" )

	# time.sleep( 1 )
	# print( "DTR False >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" )
	# ser.setDTR( False )
	# time.sleep( 0.1 )
	# print( "DTR True >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" )
	# ser.setDTR( True )

	f.close()

# end fo send_file

class FtpFsm( object ):
	def __init__( self ):
		self.__state = FTP_FSM_HEAD_S
		self.__buf = []
		self.__data_len = 0
		self.__cur_data_len = 0
		self.__checksum = 0x00
		self.__headchecksum = 0x00
		self.__recv_head_checksum = 0x00

	def get_state( self ):
		return self.__state

	def set_state( self, s ):
		self.__state = s

	def push_char( self, c ):
		if ( FTP_FSM_HEAD_S == self.__state ):
			if ( FRAME_HEAD == c ):
				self.__state = FTP_FSM_HEAD_CHECK_S
				self.__buf.clear()
				self.__checksum = 0
				self.__headchecksum = c

		elif ( FTP_FSM_HEAD_CHECK_S == self.__state ):
			self.__recv_head_checksum = c
			self.__state = FTP_FSM_LEN1_S

		elif( FTP_FSM_LEN1_S == self.__state ):
			self.__headchecksum += c
			self.__data_len = c
			self.__state = FTP_FSM_LEN2_S

		elif( FTP_FSM_LEN2_S == self.__state ):
			self.__headchecksum += c
			if ( self.__headchecksum == self.__recv_head_checksum ):
				self.__data_len += c*0xff
				self.__state = FTP_FSM_DATA_S
			else:
				self.__state = FTP_FSM_HEAD_S

		elif( FTP_FSM_DATA_S == self.__state ):
			self.__checksum += c
			self.__buf.append( c )
			if ( len(self.__buf) == self.__data_len ):
				self.__state = FTP_FSM_CHECK_S
				# print( "expect checksum %02x"%(self.__checksum & 0xFF) )

		elif( FTP_FSM_CHECK_S == self.__state ):
			if ( (self.__checksum & 0xFF) == c ):
				self.__state = FTP_FSM_END_S
			else:
				self.__state = FTP_FSM_HEAD_S
				
		elif( FTP_FSM_END_S == self.__state ):
			if ( FRAME_END == c ):
				self.__state = FTP_FSM_HEAD_S
				return self.__buf
			else:
				self.__state = FTP_FSM_HEAD_S 

	def clear_buf( self ):
		self.__buf.clear()

	def get_buf( self ):
		return self.__buf

ftp_fsm = FtpFsm()

# clear all the data in serial buffer
# time.sleep( 1 )
ser.read( ser.inWaiting() )
# time.sleep( 2 )
t = threading.Thread(target = send_task)
t.start()

while( True ):
	if ( ser.inWaiting() ):
		r_b = ser.read( ser.inWaiting() )
		for c in r_b:
			print( "%c"%(c), end='' )
			buf_list = ftp_fsm.push_char( c )
			if ( type(buf_list) == list ):
				print( " #################################### " )
				print( buf_list )
				# protocol id is 0x01 and command status code is 0x00
				if ( 0x01 == buf_list[0] and 0x00 == buf_list[6] ):
					condition.acquire()
					condition.notify( 1 )
					condition.release()
					pass
				ftp_fsm.clear_buf()
			#if ( is_display(c) ):
			#	print( "%c"%(c), end='' )
			#else:
			#	print( "%02x"%(c), end=' ' )

print( "***********************************************************************" )

# t.stop()
