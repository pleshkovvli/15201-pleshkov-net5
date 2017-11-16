#ifndef NET5_PROTOCOL_H
#define NET5_PROTOCOL_H

#define MSG_LEN 4
#define UUID_LEN 16
#define LEN_LEN 2

#define CONF_LEN (MSG_LEN + UUID_LEN)
#define READ_LEN (CONF_LEN + LEN_LEN)

#define NEW_MSG "_NEW"
#define MORE_MSG "MORE"
#define MATCH_MSG "MTCH"
#define ACK_MSG "_ACK"

#define DONE_MSG "DONE"
#define DO_MSG "__DO"

#endif //NET5_PROTOCOL_H
