#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <k4a/k4a.h>
#include <k4abt.h>
using namespace std;
#include <string>
#include "main.h"
#include "WebsocketClient.h"
#include "HttpClient.h"
#include "windows.h"
#include "time.h"

#include <fstream> 
#include <iostream>
#include <io.h>
#include<direct.h>

#define VERIFY(result, error)                                                                            \
    if(result != K4A_RESULT_SUCCEEDED)                                                                   \
    {                                                                                                    \
        printf("%s \n - (File: %s, Function: %s, Line: %d)\n", error, __FILE__, __FUNCTION__, __LINE__); \
        exit(1);                                                                                         \
    }                                                                                                    \

string print_body_information(k4abt_body_t body /*, FILE * fp*/)
{

	string result = "";
	int sum = 0;
	for (int i = 0; i < (int)K4ABT_JOINT_COUNT; i++)
	{
		char buf[1024];
		k4a_float3_t position = body.skeleton.joints[i].position;
		k4a_quaternion_t orientation = body.skeleton.joints[i].orientation;
		k4abt_joint_confidence_level_t confidence_level = body.skeleton.joints[i].confidence_level;
		int len = sprintf_s(buf, 1024, "\"Joint_%d\":\"{\\\"position\\\":\\\"(%f, %f, %f)\\\",\\\"rotation\\\":\\\"(%f, %f, %f,%f)\\\"}\"", i, position.v[0], position.v[1], position.v[2], orientation.v[0], orientation.v[1], orientation.v[2], orientation.v[3]);
		sum += len;
		result += buf;
		result += ",";
	}
	return result;
}

void print_body_index_map_middle_line(k4a_image_t body_index_map)
{
	uint8_t* body_index_map_buffer = k4a_image_get_buffer(body_index_map);

	// Given body_index_map pixel type should be uint8, the stride_byte should be the same as width
	// TODO: Since there is no API to query the byte-per-pixel information, we have to compare the width and stride to
	// know the information. We should replace this assert with proper byte-per-pixel query once the API is provided by
	// K4A SDK.
	assert(k4a_image_get_stride_bytes(body_index_map) == k4a_image_get_width_pixels(body_index_map));

	int middle_line_num = k4a_image_get_height_pixels(body_index_map) / 2;
	body_index_map_buffer = body_index_map_buffer + middle_line_num * k4a_image_get_width_pixels(body_index_map);

	printf("BodyIndexMap at Line %d:\n", middle_line_num);
	for (int i = 0; i < k4a_image_get_width_pixels(body_index_map); i++)
	{
		printf("%u, ", *body_index_map_buffer);
		body_index_map_buffer++;
	}
	printf("\n");
}






int main()
{
	int frame_num = 1;
	k4a_device_configuration_t device_config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
	device_config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;

	k4a_device_t device;
	VERIFY(k4a_device_open(0, &device), "Open K4A Device failed!");
	VERIFY(k4a_device_start_cameras(device, &device_config), "Start K4A cameras failed!");

	k4a_calibration_t sensor_calibration;
	VERIFY(k4a_device_get_calibration(device, device_config.depth_mode, K4A_COLOR_RESOLUTION_OFF, &sensor_calibration),
		"Get depth camera calibration failed!");

	k4abt_tracker_t tracker = NULL;
	k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
	VERIFY(k4abt_tracker_create(&sensor_calibration, tracker_config, &tracker), "Body tracker initialization failed!");

	int frame_count = 0;
	int interval = 10;

	int sequence = 1;
	//string host = "127.0.0.1";
	//ip of hololens emulator
	//string host = "172.31.17.236";
	//ip of hololens
	string host = "192.168.1.25";
	string port = "9005";
	string url = "/";
	//WebsocketClient client(host, port, url);
	HttpClient client(host, port, url);
	string prefix = "{\"list\":[";
	string postfix= "]}";
	string msg = "";

	clock_t start, end;
	start = clock();

	SYSTEMTIME sys;
	GetLocalTime(&sys);

	//path of the output file
	string parent_directory = "D:/data/kinect/src/";
	parent_directory = parent_directory.append(to_string(sys.wDay).c_str());
	if (_access(parent_directory.c_str(), 0) == -1) {
		//if the file does not exist, create it
		int flag = _mkdir(parent_directory.c_str());
		cout << "make successfully" << endl;
	}

	string file_path = parent_directory.append("/frames").append(to_string(sys.wMilliseconds)).append(".json");
	std::ofstream os(file_path);	
	

	try {
		os << prefix;
		do
		{

			k4a_capture_t sensor_capture;
			k4a_wait_result_t get_capture_result = k4a_device_get_capture(device, &sensor_capture, K4A_WAIT_INFINITE);

			if (get_capture_result == K4A_WAIT_RESULT_SUCCEEDED)
			{

				k4a_wait_result_t queue_capture_result = k4abt_tracker_enqueue_capture(tracker, sensor_capture, K4A_WAIT_INFINITE);

				k4a_capture_release(sensor_capture);
				if (queue_capture_result == K4A_WAIT_RESULT_TIMEOUT)
				{
					// It should never hit timeout when K4A_WAIT_INFINITE is set.
					printf("Error! Add capture to tracker process queue timeout!\n");
					break;
				}
				else if (queue_capture_result == K4A_WAIT_RESULT_FAILED)
				{
					printf("Error! Add capture to tracker process queue failed!\n");
					break;
				}

				k4abt_frame_t body_frame = NULL;
				k4a_wait_result_t pop_frame_result = k4abt_tracker_pop_result(tracker, &body_frame, K4A_WAIT_INFINITE);
				if (pop_frame_result == K4A_WAIT_RESULT_SUCCEEDED)
				{
					uint32_t num_bodies = k4abt_frame_get_num_bodies(body_frame);
					if (num_bodies == 0) {
						continue;
					}
					frame_count++;
					msg += "{";

					for (uint32_t i = 0; i < num_bodies; i++)
					{
						if (i > 0) {
							break;
						}
						k4abt_body_t body;
						VERIFY(k4abt_frame_get_body_skeleton(body_frame, i, &body.skeleton), "Get body from body frame failed!");
						body.id = k4abt_frame_get_body_id(body_frame, i);

						msg += print_body_information(body);
					}
					char buf[1024];
					sprintf_s(buf, 1024, "\"Sequence\":\"%d\"}", frame_count);
					msg += buf;
					msg += ",";

					k4a_image_t body_index_map = k4abt_frame_get_body_index_map(body_frame);
					if (body_index_map != NULL)
					{
						print_body_index_map_middle_line(body_index_map);
						k4a_image_release(body_index_map);
					}
					else
					{
						printf("Error: Fail to generate bodyindex map!\n");
					}

					k4abt_frame_release(body_frame);
				}
				else if (pop_frame_result == K4A_WAIT_RESULT_TIMEOUT)
				{
					//  It should never hit timeout when K4A_WAIT_INFINITE is set.
					printf("Error! Pop body frame result timeout!\n");
					break;
				}
				else
				{
					printf("Pop body frame result failed!\n");
					break;
				}
			}
			else if (get_capture_result == K4A_WAIT_RESULT_TIMEOUT)
			{
				// It should never hit time out when K4A_WAIT_INFINITE is set.
				printf("Error! Get depth frame time out!\n");
				continue;
			}
			else
			{
				printf("Get depth capture returned error: %d\n", get_capture_result);
				continue;
			}

			end = clock();
			if (end - start >= interval) {
				msg.erase(msg.end() - 1);			
				if (!os) { std::cerr << "Error writing to ..." << std::endl; }
				else {
					os << msg;
					os << ",";
					os.flush();
				}
				string datagram = prefix.append(msg).append(postfix);
				//send notification
				client.post(datagram);
				start = clock();
				sequence++;
				msg.clear();
				prefix = "{\"list\":[";
			}

		} while (true);
	}
	catch (exception e) {
		k4abt_tracker_shutdown(tracker);
		k4abt_tracker_destroy(tracker);
		k4a_device_stop_cameras(device);
		k4a_device_close(device);
		if (os) {
			os.flush();
			os.close();
		}
	}

	printf("Finished body tracking processing!\n");
	k4abt_tracker_shutdown(tracker);
	k4abt_tracker_destroy(tracker);
	k4a_device_stop_cameras(device);
	k4a_device_close(device);
	if (os) {
		os.flush();
		os.close();
	}

	return 0;
}