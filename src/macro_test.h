#ifndef MACRO_TEST_H_
#define MACRO_TEST_H_

/// Creates a class named with `classname` used as a TestClass in which test
/// cases uses a file with directory_path + filename0 and directory_path +
/// filename1 The content of the file is `file_content`
/// e.g. TWO_FILE_EXISTENT_TEST(FileTest, "hello world!", "good night");
#define TWO_FILE_EXISTENT_TEST(classname, file_content0, file_content1)        \
    class classname : public ::testing::Test {                                 \
      protected:                                                               \
        classname()                                                            \
            : directory_path("parent_dir/"), filename0("filename0"),           \
              filename1("filename1") {                                         \
            if (!std::filesystem::exists(directory_path)) {                    \
                std::filesystem::create_directories(directory_path);           \
            }                                                                  \
                                                                               \
            std::ofstream file0(directory_path + filename0);                   \
            if (file0.is_open()) {                                             \
                file0 << file_content0;                                        \
                file0.close();                                                 \
            }                                                                  \
            std::ofstream file1(directory_path + filename1);                   \
            if (file1.is_open()) {                                             \
                file1 << file_content1;                                        \
                file1.close();                                                 \
            }                                                                  \
        }                                                                      \
                                                                               \
        virtual ~classname() override {                                        \
            std::filesystem::remove_all(directory_path);                       \
        }                                                                      \
                                                                               \
        const std::string directory_path;                                      \
        const std::string filename0, filename1;                                \
    }

/// Creates a class named with `classname` used as a TestClass in which test
/// cases uses a file with directory_path + filename. The content of the file is
/// `file_content`
/// e.g. FILE_EXISTENT_TEST(FileTest, "hello world!");
#define FILE_EXISTENT_TEST(classname, file_content)                            \
    class classname : public ::testing::Test {                                 \
      protected:                                                               \
        classname() : directory_path("parent_dir/"), filename("filename") {    \
            if (!std::filesystem::exists(directory_path)) {                    \
                std::filesystem::create_directories(directory_path);           \
            }                                                                  \
                                                                               \
            std::ofstream file(directory_path + filename);                     \
            if (file.is_open()) {                                              \
                file << file_content;                                          \
                file.close();                                                  \
            }                                                                  \
        }                                                                      \
                                                                               \
        virtual ~classname() override {                                        \
            std::filesystem::remove_all(directory_path);                       \
        }                                                                      \
                                                                               \
        const std::string directory_path;                                      \
        const std::string filename;                                            \
    }

/// Creates a class named with `classname` used as a TestClass in which test
/// cases don't have a file with directory_path + non_existent_filename.
/// e.g. FILE_NONEXISTENT_TEST(FileTest,
#define FILE_NONEXISTENT_TEST(classname)                                       \
    class classname : public ::testing::Test {                                 \
      protected:                                                               \
        classname()                                                            \
            : directory_path("non_existent_directory/"),                       \
              non_existent_filename("non_existent_filename") {                 \
            if (std::filesystem::exists(directory_path +                       \
                                        non_existent_filename)) {              \
                remove((directory_path + non_existent_filename).c_str());      \
            }                                                                  \
        }                                                                      \
                                                                               \
        virtual ~classname() override {                                        \
            if (std::filesystem::exists(directory_path)) {                     \
                std::filesystem::remove_all(directory_path);                   \
            }                                                                  \
        }                                                                      \
                                                                               \
        const std::string directory_path;                                      \
        const std::string non_existent_filename;                               \
    }

#endif