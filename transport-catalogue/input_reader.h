#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

namespace transport_catalogue {

    namespace detail {

        struct CommandDescription {
            // ����������, ������ �� ������� (���� command ��������)
            explicit operator bool() const {
                return !command.empty();
            }

            bool operator!() const {
                return !operator bool();
            }

            std::string command;      // �������� �������
            std::string id;           // id �������� ��� ���������
            std::string description;  // ��������� �������
        };
    }

    namespace reader {

        class InputReader {
        public:
            /**
             * ������ ������ � ��������� CommandDescription � ��������� ��������� � commands_
             */
            void ParseLine(std::string_view line);

            /**
             * ��������� ������� ������������ ����������, ��������� ������� �� commands_
             */
            void ApplyCommands(TransportCatalogue& catalogue) const;

            void LoadCommands(std::istream& input, TransportCatalogue& catalogue);

        private:
            std::vector<detail::CommandDescription> commands_;
        };
    }
}