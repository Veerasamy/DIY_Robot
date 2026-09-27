# generated from rosidl_generator_py/resource/_idl.py.em
# with input from safety_manager:msg/EStopStatus.idl
# generated code does not contain a copyright notice

# This is being done at the module level and not on the instance level to avoid looking
# for the same variable multiple times on each instance. This variable is not supposed to
# change during runtime so it makes sense to only look for it once.
from os import getenv

ros_python_check_fields = getenv('ROS_PYTHON_CHECK_FIELDS', default='')


# Import statements for member types

import builtins  # noqa: E402, I100

import rosidl_parser.definition  # noqa: E402, I100


class Metaclass_EStopStatus(type):
    """Metaclass of message 'EStopStatus'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('safety_manager')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'safety_manager.msg.EStopStatus')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__msg__e_stop_status
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__msg__e_stop_status
            cls._CONVERT_TO_PY = module.convert_to_py_msg__msg__e_stop_status
            cls._TYPE_SUPPORT = module.type_support_msg__msg__e_stop_status
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__msg__e_stop_status

            from builtin_interfaces.msg import Time
            if Time.__class__._TYPE_SUPPORT is None:
                Time.__class__.__import_type_support__()

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class EStopStatus(metaclass=Metaclass_EStopStatus):
    """Message class 'EStopStatus'."""

    __slots__ = [
        '_stamp',
        '_estop_pressed',
        '_link_ok',
        '_radio_ok',
        '_heartbeat_seq',
        '_battery_millivolts',
        '_radio_rssi_dbm',
        '_check_fields',
    ]

    _fields_and_field_types = {
        'stamp': 'builtin_interfaces/Time',
        'estop_pressed': 'boolean',
        'link_ok': 'boolean',
        'radio_ok': 'boolean',
        'heartbeat_seq': 'uint8',
        'battery_millivolts': 'uint16',
        'radio_rssi_dbm': 'int8',
    }

    # This attribute is used to store an rosidl_parser.definition variable
    # related to the data type of each of the components the message.
    SLOT_TYPES = (
        rosidl_parser.definition.NamespacedType(['builtin_interfaces', 'msg'], 'Time'),  # noqa: E501
        rosidl_parser.definition.BasicType('boolean'),  # noqa: E501
        rosidl_parser.definition.BasicType('boolean'),  # noqa: E501
        rosidl_parser.definition.BasicType('boolean'),  # noqa: E501
        rosidl_parser.definition.BasicType('uint8'),  # noqa: E501
        rosidl_parser.definition.BasicType('uint16'),  # noqa: E501
        rosidl_parser.definition.BasicType('int8'),  # noqa: E501
    )

    def __init__(self, **kwargs):
        if 'check_fields' in kwargs:
            self._check_fields = kwargs['check_fields']
        else:
            self._check_fields = ros_python_check_fields == '1'
        if self._check_fields:
            assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
                'Invalid arguments passed to constructor: %s' % \
                ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        from builtin_interfaces.msg import Time
        self.stamp = kwargs.get('stamp', Time())
        self.estop_pressed = kwargs.get('estop_pressed', bool())
        self.link_ok = kwargs.get('link_ok', bool())
        self.radio_ok = kwargs.get('radio_ok', bool())
        self.heartbeat_seq = kwargs.get('heartbeat_seq', int())
        self.battery_millivolts = kwargs.get('battery_millivolts', int())
        self.radio_rssi_dbm = kwargs.get('radio_rssi_dbm', int())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.get_fields_and_field_types().keys(), self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    if self._check_fields:
                        assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.stamp != other.stamp:
            return False
        if self.estop_pressed != other.estop_pressed:
            return False
        if self.link_ok != other.link_ok:
            return False
        if self.radio_ok != other.radio_ok:
            return False
        if self.heartbeat_seq != other.heartbeat_seq:
            return False
        if self.battery_millivolts != other.battery_millivolts:
            return False
        if self.radio_rssi_dbm != other.radio_rssi_dbm:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def stamp(self):
        """Message field 'stamp'."""
        return self._stamp

    @stamp.setter
    def stamp(self, value):
        if self._check_fields:
            from builtin_interfaces.msg import Time
            assert \
                isinstance(value, Time), \
                "The 'stamp' field must be a sub message of type 'Time'"
        self._stamp = value

    @builtins.property
    def estop_pressed(self):
        """Message field 'estop_pressed'."""
        return self._estop_pressed

    @estop_pressed.setter
    def estop_pressed(self, value):
        if self._check_fields:
            assert \
                isinstance(value, bool), \
                "The 'estop_pressed' field must be of type 'bool'"
        self._estop_pressed = value

    @builtins.property
    def link_ok(self):
        """Message field 'link_ok'."""
        return self._link_ok

    @link_ok.setter
    def link_ok(self, value):
        if self._check_fields:
            assert \
                isinstance(value, bool), \
                "The 'link_ok' field must be of type 'bool'"
        self._link_ok = value

    @builtins.property
    def radio_ok(self):
        """Message field 'radio_ok'."""
        return self._radio_ok

    @radio_ok.setter
    def radio_ok(self, value):
        if self._check_fields:
            assert \
                isinstance(value, bool), \
                "The 'radio_ok' field must be of type 'bool'"
        self._radio_ok = value

    @builtins.property
    def heartbeat_seq(self):
        """Message field 'heartbeat_seq'."""
        return self._heartbeat_seq

    @heartbeat_seq.setter
    def heartbeat_seq(self, value):
        if self._check_fields:
            assert \
                isinstance(value, int), \
                "The 'heartbeat_seq' field must be of type 'int'"
            assert value >= 0 and value < 256, \
                "The 'heartbeat_seq' field must be an unsigned integer in [0, 255]"
        self._heartbeat_seq = value

    @builtins.property
    def battery_millivolts(self):
        """Message field 'battery_millivolts'."""
        return self._battery_millivolts

    @battery_millivolts.setter
    def battery_millivolts(self, value):
        if self._check_fields:
            assert \
                isinstance(value, int), \
                "The 'battery_millivolts' field must be of type 'int'"
            assert value >= 0 and value < 65536, \
                "The 'battery_millivolts' field must be an unsigned integer in [0, 65535]"
        self._battery_millivolts = value

    @builtins.property
    def radio_rssi_dbm(self):
        """Message field 'radio_rssi_dbm'."""
        return self._radio_rssi_dbm

    @radio_rssi_dbm.setter
    def radio_rssi_dbm(self, value):
        if self._check_fields:
            assert \
                isinstance(value, int), \
                "The 'radio_rssi_dbm' field must be of type 'int'"
            assert value >= -128 and value < 128, \
                "The 'radio_rssi_dbm' field must be an integer in [-128, 127]"
        self._radio_rssi_dbm = value
