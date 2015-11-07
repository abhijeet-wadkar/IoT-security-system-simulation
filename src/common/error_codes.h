/*
 * error_codes.h
 *
 *  Created on: Sep 27, 2015
 *      Author: Abhijeet Wadkar, Devendra Dahiphale
 */

#define E_SUCCESS 1
#define E_FAILURE 0
/* Memory errors -1 t0 -100 */
#define E_OUT_OF_MEMORY -1
#define E_INVALID_HANDLE -2

/* Network errors -101 to -200 */
#define E_SOCKET_CREATE_ERROR -101
#define E_SOCKET_CONNCTION_ERORR -102
#define E_SOCKET_SEND_ERROR -103
#define E_SOCKET_CONNECTION_CLOSED -104

/* Message paring errors -201 to -300*/
#define E_INVALID_MESSAGE -201



