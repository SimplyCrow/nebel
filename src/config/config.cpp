#include "config.hpp"

#include "../panic.hpp"

namespace config {

        Config::Config(std::filesystem::path root_config_file)
                : m_config{}
        {
                config::info("libconfig: {}.{}.{}", LIBCONFIGXX_VER_MAJOR, LIBCONFIGXX_VER_MINOR, LIBCONFIGXX_VER_REVISION);

                std::filesystem::path config_base_directory = root_config_file.parent_path();
                m_config.setIncludeDir(config_base_directory.c_str());
                m_config.setAutoConvert(true);
                try {
                        m_config.readFile(root_config_file.c_str());
                } catch (libconfig::ParseException &e)
                {
                        PANIC(config, "Parser error:{}:{}: {}", e.getFile(), e.getLine(), e.getError());
                } catch (libconfig::FileIOException &e)
                {
                        PANIC(config, "No such file error. Found: {}", root_config_file.string());
                }
        }

}; // namespace config
