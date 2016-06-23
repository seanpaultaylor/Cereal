#pragma once

#include <vector>
#include <string>

#include "Writer.h"
#include "Reader.h"
#include "..\Cereal.h"

#include "Field.h"
#include "Array.h"

namespace Cereal {

	class Object
	{
	private:
		byte type;
		std::string name;
		std::vector<const Array*> arrays;
		std::vector<const Field*> fields;

	public:
		Object() {};
		Object(std::string name)
		{
			this->name = name;
			this->type = DataType::DATA_OBJECT;
		}

		~Object()
		{
			for (size_t i = 0; i < arrays.size(); i++)
				delete arrays[i];

			for (size_t i = 0; i < fields.size(); i++)
				delete fields[i];
		}

		unsigned int write(byte* dest, int pointer) const
		{
			pointer = Writer::writeBytes<byte>(dest, pointer, type);
			pointer = Writer::writeBytes<std::string>(dest, pointer, name);

			pointer = Writer::writeBytes<unsigned short>(dest, pointer, (unsigned short)fields.size());

			for (const Field* field : fields)
				pointer = field->write(dest, pointer);

			pointer = Writer::writeBytes<unsigned short>(dest, pointer, (unsigned short)arrays.size());

			for (const Array* array : arrays)
				pointer = array->write(dest, pointer);

			return pointer;
		}

		inline void addField(const Field* field) { fields.push_back(field); }
		inline void addArray(const Array* array) { arrays.push_back(array); }

		inline const Field* findField(std::string name) const
		{
			for (const Field* field : fields)
				if (field->getName() == name) return field;

			return nullptr;
		}

		inline const Array* findArray(std::string name) const
		{
			for (const Array* array : arrays)
				if (array->getName() == name) return array;

			return nullptr;
		}

		void read(byte* dest, int pointer)
		{
			this->type = Reader::readBytes<byte>(dest, pointer++);

			assert(this->type == DataType::DATA_OBJECT);

			this->name = Reader::readBytes<std::string>(dest, pointer);

			pointer += sizeof(unsigned short) + name.length();

			unsigned short fieldCount = Reader::readBytes<unsigned short>(dest, pointer);

			pointer += sizeof(unsigned short);

			for (int i = 0; i < fieldCount; i++)
			{
				Field* field = new Field;

				field->read(dest, pointer);
				this->addField(field);

				pointer += field->getSize();
			}

			unsigned short arrayCount = Reader::readBytes<unsigned short>(dest, pointer);

			pointer += sizeof(unsigned short);

			for (int i = 0; i < arrayCount; i++)
			{
				Array* array = new Array;

				array->read(dest, pointer);
				this->addArray(array);

				pointer += array->getSize();
			}
		}

		const std::string& getName() const { return name; }
		byte getContainerType() const { return type; }

		inline unsigned int getSize() const
		{
			unsigned int ret = sizeof(byte) + sizeof(short) + name.length() + sizeof(short) + sizeof(short);

			for (const Field* field : fields)
				ret += field->getSize();

			for (const Array* array : arrays)
				ret += array->getSize();

			return ret;
		}

	};

}