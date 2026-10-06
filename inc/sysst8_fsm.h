/**
 * @file sysst8_fsm.h
 * @author Shang Huang
 * @brief Finite State Machine definitions and utilities for SYSST8 system
 * @version 0.1
 * @date 2026-04-16
 * @copyright MIT License
 */
#ifndef __SYSST8_FSM_H__
	#define __SYSST8_FSM_H__

	// ANCHOR - Khai báo thư viện sử dụng
	#include <stdint.h>
	#include "sysst8_conf.h"
	#include "sysst8_fcr.h"

	/** ANCHOR - Khai báo kiểu dữ liệu để quản lý tin nhắn trong hệ thống SYSST8
	 * @attention `sysst8_msg_t` được gọi ở đây để thực thi forward declaration, 
	 * 						cho phép sử dụng con trỏ đến `sysst8_msg_t` trong các khai báo sau này
	 */
	typedef enum sysst8_msg_t sysst8_msg_t;
 
	/** ANCHOR - Định nghĩa kiểu hàm xử lý trạng thái trong FSM
	 * @param msg là con trỏ đến tin nhắn được gửi đến FSM, 
	 * 				cho phép hàm xử lý trạng thái truy cập và xử lý thông tin từ tin nhắn đó 
	 * 				để thực hiện các hành động tương ứng dựa trên nội dung của tin nhắn và trạng thái hiện tại của FSM.
	 */
	typedef void (*state_handler)(sysst8_msg_t* msg);

	/** ANCHOR - Định nghĩa cấu trúc để quản lý thông tin của FSM trong hệ thống SYSST8
	 * @attention `history` không được khai báo vượt quá SYSST8_FSM_HIS_MAX 
	 * 						để đảm bảo an toàn bộ nhớ và tránh lỗi tràn bộ nhớ 
	 * 						khi lưu lịch sử trạng thái của FSM.
	 * @param state là con trỏ đến hàm xử lý trạng thái hiện tại của FSM
	 * @param history là mảng lưu trữ các hàm xử lý trạng thái trước
	 * @param history_index là chỉ số hiện tại trong mảng history
	 * @param history_count là số lượng trạng thái đã lưu trong history
	 */
	typedef struct sysst8_fsm_t {
		state_handler state;
		state_handler history[SYSST8_FSM_HIS_MAX];
		uint8_t history_index;
		uint8_t history_count;
	} sysst8_fsm_t;

	// ANCHOR - Khởi tạo FSM và thực hiện hành động INIT đầu tiên
	#define sysst8_fsm_init(me, init_func) \
	do { \
		(me)->state = (state_handler)(init_func); \
		(me)->history_index = 0; \
		(me)->history_count = 0; \
		memset((me)->history, 0, sizeof((me)->history)); \
		sysst8_msg_t* m = sysst8_msg_alloc(0, SYSST8_FSM_SIG_INIT, 0); \
		if (m != NULL) { \
			(init_func)(m); /* Gửi tín hiệu INIT đến trạng thái khởi tạo của FSM */ \
		} \
	} while(0)
	//NOTE - Add free m after use to avoid memory leak

	/** ANCHOR - Khai báo hàm để xử lý tin nhắn và điều hướng trạng thái trong FSM
	 * @param me chỉ trạng thái hiện tại của FSM
	 * @param msg chỉ con trỏ đến tin nhắn được gửi đến FSM
	 * @note Hàm này được dùng để điều phối các tin nhắn đến FSM và gọi trong while loop của chương trình chính while(1) để xử lý các tin nhắn đến FSM.
	 */
	static inline void sysst8_fsm_dispatch(sysst8_fsm_t* me, sysst8_msg_t* msg) {
		if (me && me->state && msg) {
			me->state(msg);
		} else if (me && msg && !me->state) {
			SYSST8_FCR_RAISE(SYSST8_FCR_SM_NULL_HANDLER); // FSM chưa được sysst8_fsm_init() hoặc bị go_next(NULL)
		}
	}

	/** ANCHOR - Hàm để chuyển đổi trạng thái của FSM
	 * @param me chỉ trạng thái hiện tại của FSM
	 * @param target chỉ hàm xử lý trạng thái mục tiêu mà FSM sẽ chuyển đến
	 */
	void sysst8_fsm_go_next(sysst8_fsm_t* me, state_handler target);

	/** ANCHOR - Hàm để quay lại trạng thái trước đó của FSM dựa trên lịch sử đã lưu
	 * @param me chỉ trạng thái hiện tại của FSM, hàm sẽ sử dụng thông tin trong `history` để quay lại trạng thái trước đó
	 */
	void sysst8_fsm_go_back(sysst8_fsm_t* me);

	/** ANCHOR - Hàm để giữ nguyên trạng thái hiện tại của FSM
	 * @param me chỉ trạng thái hiện tại của FSM
	 */
	void sysst8_fsm_stay(sysst8_fsm_t* me);

#endif //__SYSST8_FSM_H__
