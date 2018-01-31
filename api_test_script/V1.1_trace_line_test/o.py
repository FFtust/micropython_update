r_on_white = 200
		r_on_blace = 100
		back_to_line_delay = 0.1
		diff_err_squ = 20
		find_road_speed = 40
		if ( rocky.red() > r_on_white ):
			on_white_cnt += 1
		else:
			on_white_cnt = 0
		# print( "R: %d"%(rocky.red()) )
		if ( on_white_cnt > 3 ):
			gray_err_diff_sum = 0
			gray_err_diff_squ_sum = 0
			for i in range(record_cnt):
				gray_err_diff_sum += gray_diff_err[i]
				gray_err_diff_squ_sum += gray_diff_err_squ[i]

			if( gray_err_diff_squ_sum > diff_err_squ ):
				print( "gray_1 change much than gray_2 ########  LEFT CROSSING" )
				rocky.left( trace_speed )
				while( True ):
					if rocky.red() < r_on_blace:
						break
				rocky.left( trace_speed )
				time.sleep( back_to_line_delay )
				rocky.drive( trace_speed, trace_speed )
						
			elif ( gray_err_diff_squ_sum < -diff_err_squ ):
				print( "gray_2 change much than gray_1 ########  RIGHT CROSSING" )
				rocky.right( find_road_speed )
				while( True ):
					if rocky.red() < r_on_blace:
						break
				rocky.right( find_road_speed )
				time.sleep( back_to_line_delay )
				rocky.drive( trace_speed, trace_speed )

			else:
				print( "Both change the same" )
				if ( gray_err_diff_sum < 0 ):
					print( "avg gray_1 has the lower gray, ########  LEFT CROSSING" )
					rocky.left( find_road_speed )
					while( True ):
						if rocky.red() < r_on_blace:
							break
					rocky.left( find_road_speed )
					time.sleep( back_to_line_delay )
					rocky.drive( trace_speed, trace_speed )

				elif ( gray_err_diff_sum > 0 ):
					print( "avg gray_2 has the lower gray, ########  RIGHT CROSSING" )
					rocky.right( find_road_speed )
					while( True ):
						if rocky.red() < r_on_blace:
							break
					rocky.right( find_road_speed )
					time.sleep( back_to_line_delay )
					rocky.drive( trace_speed, trace_speed )

				else:
					print( "avg gray has the same, CAN NOT make a dicision, backward" )
					rocke.back(find_road_speed)
					while( True ):
						if rocky.red() < r_on_blace:
							break
					time.sleep( back_to_line_delay )