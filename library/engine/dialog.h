#ifndef __ENGINE_FILEDIALOG_H__
#define __ENGINE_FILEDIALOG_H__

namespace Engine {

    namespace Dialog {

        std::string EnvironmentVariable(const std::string &variable);

        class ColorDialog {
        public:
            ColorDialog(sf::Color col);
            virtual ~ColorDialog();

            static COLORREF customColors[16];
            virtual sf::Color result(){ return _result; }

        protected:
            sf::Color _result;
        private:
            CHOOSECOLOR cc;
        };


        class FileDialog {
        public:
            struct Filter {
                const char* label;
                const char* filter;
            };

            FileDialog(std::string &filePath, const std::string &path, const std::vector<Filter> &flt, int defaultFilter, DWORD flags, const std::string &title);
            virtual ~FileDialog();

        protected:
            void store(bool success);
            OPENFILENAME fileCtx;

        private:
            std::string& filePathRef;
            std::string strFilter;
            char szFile[MAX_PATH];
        };

        class SaveDialog: public FileDialog {
        public:
            SaveDialog(std::string &filePath,
                       const std::string &path="",
                       const std::vector<FileDialog::Filter> &flt = {{"All Files (*.*)","*.*"}},
                       int defaultFilter=0,
                       DWORD flags=OFN_OVERWRITEPROMPT,
                       const std::string &title="Save File");
            virtual ~SaveDialog();
        };

        class LoadDialog: public FileDialog {
        public:
            LoadDialog(std::string &filePath,
                       const std::string &path="",
                       const std::vector<FileDialog::Filter> &flt = {{"All Files (*.*)","*.*"}},
                       int defaultFilter=0,
                       DWORD flags=OFN_FILEMUSTEXIST,
                       const std::string &title="Open File");
            virtual ~LoadDialog();
        };

        class MessageDialog {
        public:
            MessageDialog(const std::string &message, const std::string &title="", UINT msgIcon=0);
            virtual ~MessageDialog();
            virtual int result(){ return _result; }
        protected:
            int _result;
        };


        class QuestionDialog: public MessageDialog {
        public:
            QuestionDialog(const std::string &message="",
                           const std::string &title="",
                           UINT msgIcon=0);
            virtual ~QuestionDialog();
            int result();
        };

    }

}

#endif // __ENGINE_FILEDIALOG_H__
