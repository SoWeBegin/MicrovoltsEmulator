
#ifndef CDB_MANAGER
#define CDB_MANAGER

#include <unordered_map>
#include <initializer_list>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <vector>
#include <optional>
#include <cstring>
#include "visit_struct/visit_struct.hpp"


namespace Common
{
	namespace ConstantDatabase
	{
        template<typename U, typename T>
        constexpr bool containsHelper(U inner, T val)
        {
            if constexpr (std::same_as<const char*, T>)
            {
                return false;
            }
            else return inner == val;
        }

        template<typename T>
        constexpr bool containsHelper(const char* inner, T val)
        {
            if constexpr (std::same_as<const char*, T>)
            {
                return std::string{ inner } == std::string{ val };
            }
            else return false;
        }

		template<typename T>
		class Cdb
		{
		private:
			std::unordered_map<std::size_t, T> m_entries;
			std::vector<std::string> m_keys;
			std::size_t m_entry_num = 0;

			template<typename U>
			friend class Cdb;

		public:
			constexpr Cdb()
			{
				addKeysFromStruct(T{});
			}

			constexpr explicit Cdb(const std::filesystem::path& directory, const std::string& file_name)
			{
				addKeysFromStruct(T{});
				parse(directory, file_name);
			}

			constexpr void parse(const std::filesystem::path& directory, const std::string& file_name)
			{
				const std::filesystem::path file_path = directory / file_name;
				std::ifstream input{ file_path.string(), std::ios::binary };

				// 1) Retrieve the total number of keys
				std::uint32_t total_keys = 0;
				input.read(reinterpret_cast<char*>(&total_keys), sizeof(std::uint32_t));

				// Skip header
				const std::size_t header_bytes_num = 4 + total_keys * 34;
				input.seekg(header_bytes_num, std::ios::beg);

				// 2) Retrieve the entries
				std::size_t i = 0;
				while (input.good() && input.peek() != EOF)
				{
					std::array<char, sizeof(T)> buffer;
					input.read(buffer.data(), sizeof(T));

					T value;
					std::memcpy(&value, buffer.data(), sizeof(T));

					m_entries[m_entry_num++] = value;
					i = i >= total_keys ? 0 : i;
				}
			}

			constexpr void reset()
			{
				m_entries.clear();
				m_keys.clear();
				m_entry_num = 0;
			}

			constexpr void publish(const std::filesystem::path& directory, const std::string& file_name) const
			{
				const std::filesystem::path file_path = directory / file_name;
				std::ofstream output{ file_path.string(), std::ios::binary };

				// 1) Add the total keys value
				const std::size_t size = m_keys.size();
				output.write(reinterpret_cast<const char*>(&size), 4);

				// 2) Add the keys
				for (std::string key : m_keys)
				{
					key.resize(30, '\0');
					output.write(key.c_str(), 30);
				}

				// 3) Add the sizes
				publishTypeHelper<0>(output);

				// 4) Add the values
				for (const auto& [entry_num, entry] : m_entries)
				{
					output.write(reinterpret_cast<const char*>(&entry), sizeof(T));
				}
			}

			//template<typename T>
			constexpr std::size_t countMatches(const std::string& key, T value) const
			{
				std::size_t occurrences = 0;
				for (const auto& [entry, inner_struct] : m_entries)
				{
					// Leave this as it is. The "!found" check is intentional, and so are the two "check" functions.
					// Without those this snippet wouldn't work due to visit_struct macro weirdness
					visit_struct::for_each(inner_struct, [&](const char* name, const auto& inner_value)
						{
							if (std::strcmp(name, key.c_str()) == 0)
							{
								if (contains_helper(inner_value, value))
								{
									++occurrences;
								}
							}

						});
				}
				return occurrences;
			}

			//template<typename T>
			constexpr std::size_t contains(const std::string& keys, T value) const
			{
				return count_matches(keys, value) > 0;
			}

		private:
			void copyAndAddValue(const auto& lhs_entries)
			{
				for (const auto& [entry_num, entry] : lhs_entries)
				{
					T t{};
					std::memcpy(&t, &entry, sizeof(entry));
					m_entries[m_entry_num++] = t;
				}
			}

		public:
			// Add a new key and value to the Cdb instance
			template<typename T2>
			Cdb<T2> addKeys()
			{
				Cdb<T2> new_cdb{};
				new_cdb.copy_and_add_value(m_entries);
				return new_cdb;
			}

			//template<typename T>
			constexpr void replaceValue(const std::string& key, T value, T new_value)
			{
				for (auto& [entry, inner_struct] : m_entries)
				{
					// typename T on purpose; using auto& here stops working due to visit_struct weirdness!
					visit_struct::for_each(inner_struct, [&](const char* name, T & inner_value)
					{
						// if constexpr necessary, otherwise visit_struct macro hell doesn't compile this
						if constexpr (not std::convertible_to<T, const char*>)
						{
							if (std::strcmp(name, key.c_str()) == 0 && inner_value == value)
							{
								inner_value = new_value;
							}
						}
					});
				}
			}

			constexpr void replaceValue(const std::string& key, const char* value, const char* new_value)
			{
				for (auto& [entry, inner_struct] : m_entries)
				{
					// typename T on purpose; using auto& here stops working due to visit_struct weirdness!
					visit_struct::for_each(inner_struct, [&](const char* name, T & inner_value)
					{
						// if constexpr necessary, otherwise visit_struct macro hell doesn't compile this
						if constexpr (std::convertible_to<T, const char*>)
						{
							if (std::strcmp(name, key.c_str()) == 0 && std::strcmp(inner_value, value) == 0)
							{
								strncpy_s(inner_value, sizeof(inner_value), new_value, sizeof(inner_value) - 1);
							}
						}
					});
				}
			}

			[[nodiscard]] constexpr std::optional<T> getEntry(const std::string& key, auto value) const
			{
				bool found = false;
				std::optional<T> ret;
				for (const auto& [entry, inner_struct] : m_entries)
				{
					// Leave this as it is. The "!found" check is intentional, and so are the two "check" functions.
					// Without those this snippet wouldn't work due to visit_struct macro weirdness
					visit_struct::for_each(inner_struct, [&](const char* name, const auto& inner_value)
						{
							if (std::strcmp(name, key.c_str()) == 0 && !found)
							{
								found = containsHelper(inner_value, value);
								if (found)
								{
									ret.emplace(inner_struct);
								}
							}

						});
				}
				return found ? ret : std::nullopt;
			}

			[[nodiscard]] constexpr std::optional<T> getEntry(std::size_t entry_num) const
			{
				if (m_entries.contains(entry_num))
				{
					return m_entries.at(entry_num);
				}
				return std::nullopt;
			}

			template<typename... S>
			requires (std::same_as<S, T> and ...)
			constexpr void addEntries(const S&... s)
			{
				(add_entry(s), ...);
			}

			constexpr std::unordered_map<std::size_t, T> getEntries() const
			{
				return m_entries;
			}

			constexpr std::size_t total_entries() const
			{
				return m_entries.size();
			}

			template<typename Stream>
			constexpr void printEntries(Stream& stream) const
			{
				for (const auto& [entry_num, entry] : m_entries)
				{
					stream << "[Entry " << entry_num << "]\n";

					visit_struct::for_each(entry, [&](const char* name, const auto& value)
						{
							stream << "  " << name << " = " << value << '\n';
						});

					stream << '\n';
				}
			}


		private:
			// The following two useless functions are intentional. Without them visit_struct::for_each doesn't work properly for some obscure reason (probably due to macros)

			template<std::size_t I>
			constexpr void publishTypeHelper(std::ofstream& output) const
			{
				if constexpr (I < visit_struct::field_count<T>())
				{
					output.write(reinterpret_cast<const char*>(&asLvalue(sizeof(visit_struct::type_at<I, T>))), 4);
					publishTypeHelper<I + 1>(output);
				}
			}

			constexpr void addEntry(const T& t)
			{
				m_entries[m_entry_num++] = t;
			}

			constexpr void addKeysFromStruct(const T& t)
			{
				visit_struct::for_each(t, [&](const char* name, auto unused)
					{
						m_keys.push_back(name);
					});
			}

			//template <typename T>
			constexpr T& asLvalue(T&& x) const
			{
				return x;
			}
		};
	}
}

#endif