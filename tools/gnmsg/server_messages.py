from read_values import (
    call_reader_function,
    read_int_value,
    read_byte_value,
    read_cacheable,
    parse_key_or_value,
)


def read_bucket_count(message_bytes, offset):
    object_part = {}
    (object_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (object_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (object_part["Data"], offset) = read_cacheable(message_bytes, offset)

    return (object_part, offset)


def read_partition_attributes(properties, message_bytes, offset):
    if properties["Parts"] != 2 and properties["Parts"] != 4:
        raise Exception(
            "Don't know how to parse a RESPONSE_CLIENT_PARTITION_ATTRIBUTES message with "
            + properties["Parts"]
            + " parts (should have 2 or 4 only)."
        )

    (properties["BucketCount"], offset) = read_bucket_count(message_bytes, offset)
    (properties["ColocatedWith"], offset) = parse_key_or_value(message_bytes, offset)
    # TODO: parse parts 3 and 4 (partition resolver and list of partition attributes), if they exist


server_message_parsers = {
    "RESPONSE_CLIENT_PARTITION_ATTRIBUTES": read_partition_attributes,
}


def parse_server_message(properties, message_bytes):
    offset = 0
    if properties["Type"] in server_message_parsers.keys():
        server_message_parsers[properties["Type"]](properties, message_bytes, offset)
