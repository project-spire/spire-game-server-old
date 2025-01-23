target_sources(base_lib PUBLIC
    client.cpp
    client.hpp
    connection.cpp
    connection.hpp
    heartbeater.cpp
    heartbeater.hpp
    message.cpp
    message.hpp
    message_handler.hpp
    room.cpp
    room.hpp
    server.cpp
    server.hpp

    handler/waiting_room_message_handler.cpp
    handler/waiting_room_message_handler.hpp
)