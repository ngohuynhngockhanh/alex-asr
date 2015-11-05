//
// Created by zilka on 11/5/15.
//

#include "pykaldi2_decoder/pykaldi2_decoder_config.h"

namespace kaldi {
    PyKaldi2DecoderConfig::PyKaldi2DecoderConfig() :
            lda_mat(NULL),
            cmvn_mat(NULL),
            ivector_extraction_info(NULL),
            bits_per_sample(16),
            use_ivectors(false),
            use_cmvn(false),
            use_pitch(false),
            cfg_decoder("cfg.decoder"),
            cfg_decodable("cfg.decodable"),
            cfg_mfcc("cfg.mfcc"),
            cfg_cmvn("cfg.cmvn"),
            cfg_splice("cfg.splice"),
            cfg_endpoint("cfg.endpoint"),
            cfg_ivector("cfg.ivector"),
            cfg_pitch("cfg.pitch")
    {
        decodable_opts.acoustic_scale = 0.1;
        splice_opts.left_context = 3;
        splice_opts.right_context = 3;
    }

    PyKaldi2DecoderConfig::~PyKaldi2DecoderConfig() {
        delete lda_mat;
        delete cmvn_mat;
        delete ivector_extraction_info;
    }

    void PyKaldi2DecoderConfig::Register(ParseOptions *po) {
        po->Register("model", &model_rxfilename, "Accoustic model filename.");
        po->Register("hclg", &fst_rxfilename, "HCLG FST filename.");
        po->Register("words", &words_rxfilename, "Word to ID mapping filename.");
        po->Register("mat_lda", &lda_mat_rspecifier, "LDA matrix filename.");
        po->Register("mat_cmvn", &fcmvn_mat_rspecifier, "CMVN matrix filename.");
        po->Register("use_ivectors", &use_ivectors, "Are we using ivector features?");
        po->Register("use_cmvn", &use_cmvn, "Are we using cmvn transform?");
        po->Register("use_pitch", &use_pitch, "Are we using pitch feature?");
        po->Register("bits_per_sample", &bits_per_sample, "Bits per sample for input.");

        po->Register("cfg_decoder", &cfg_decoder, "");
        po->Register("cfg_decodable", &cfg_decodable, "");
        po->Register("cfg_mfcc", &cfg_mfcc, "");
        po->Register("cfg_cmvn", &cfg_cmvn, "");
        po->Register("cfg_splice", &cfg_splice, "");
        po->Register("cfg_endpoint", &cfg_endpoint, "");
        po->Register("cfg_ivector", &cfg_ivector, "");
        po->Register("cfg_pitch", &cfg_pitch, "");
    }

    void PyKaldi2DecoderConfig::LoadConfigs(const string cfg_file) {
        std::string model_path("");

        ParseOptions po("");
        Register(&po);

        KALDI_VLOG(2) << "Reading master config file: " << cfg_file;
        po.ReadConfigFile(cfg_file);

        LoadConfig(cfg_decoder, &decoder_opts);
        LoadConfig(cfg_decodable, &decodable_opts);
        LoadConfig(cfg_mfcc, &mfcc_opts);
        LoadConfig(cfg_cmvn, &cmvn_opts);
        LoadConfig(cfg_splice, &splice_opts);
        LoadConfig(cfg_endpoint, &endpoint_config);
        LoadConfig(cfg_ivector, &ivector_config);
        LoadConfig(cfg_pitch, &pitch_opts);
        LoadConfig(cfg_pitch, &pitch_process_opts);

        InitAux();
    }

    void PyKaldi2DecoderConfig::InitAux() {
        LoadLDA();

        if (use_cmvn) {
            LoadCMVN();
        }

        if (use_ivectors) {
            LoadIvector();
        }
    }

    void PyKaldi2DecoderConfig::LoadLDA() {
        KALDI_VLOG(2) << "Loading LDA matrix.";
        bool binary_in;
        Input ki(lda_mat_rspecifier, &binary_in);

        KALDI_PARANOID_ASSERT(lda_mat == NULL);
        lda_mat = new Matrix<BaseFloat>();
        lda_mat->Read(ki.Stream(), binary_in);
    }

    void PyKaldi2DecoderConfig::LoadCMVN() {
        KALDI_VLOG(2) << "Loading global CMVN stats.";
        bool binary_in;
        Input ki(fcmvn_mat_rspecifier, &binary_in);

        KALDI_PARANOID_ASSERT(cmvn_mat == NULL);
        cmvn_mat = new Matrix<double>();
        cmvn_mat->Read(ki.Stream(), binary_in);
    }

    void PyKaldi2DecoderConfig::LoadIvector() {
        KALDI_LOG << "Loading IVector extraction info.";
        ivector_extraction_info = new OnlineIvectorExtractionInfo(ivector_config);
    }

    template<typename C>
    void PyKaldi2DecoderConfig::LoadConfig(string file_name, C *opts) {
        if (FileExists(file_name)) {
            ReadConfigFromFile(file_name, opts);
            KALDI_VLOG(2) << "Config loaded: " << file_name;
        } else {
            KALDI_VLOG(2) << "Config not found: " << file_name;
        }
    }

    bool PyKaldi2DecoderConfig::FileExists(string strFilename) {
        struct stat stFileInfo;
        bool blnReturn;
        int intStat;

        // Attempt to get the file attributes
        intStat = stat(strFilename.c_str(), &stFileInfo);
        if (intStat == 0) {
            // We were able to get the file attributes
            // so the file obviously exists.
            blnReturn = true;
        } else {
            // We were not able to get the file attributes.
            // This may mean that we don't have permission to
            // access the folder which contains this file. If you
            // need to do that level of checking, lookup the
            // return values of stat which will give you
            // more details on why stat failed.
            blnReturn = false;
        }

        return blnReturn;
    }

    bool PyKaldi2DecoderConfig::Check() {
        bool res = true;
        res |= OptionCheck(use_ivectors && cfg_ivector == "",
                           "You have to specify --cfg_ivector if you want to use ivectors.");
        res |= OptionCheck(use_cmvn && fcmvn_mat_rspecifier == "",
                           "You have to specify --cfg_cmvn if you want to use CMVN.");
        res |= OptionCheck(use_pitch && cfg_pitch == "",
                           "You have to specify --cfg_pitch if you want to use pitch.");

        res |= OptionCheck(model_rxfilename == "",
                           "You have to specify --model.");

        res |= OptionCheck(fst_rxfilename == "",
                           "You have to specify --hclg.");

        res |= OptionCheck(words_rxfilename == "",
                           "You have to specify --words.");

        res |= OptionCheck(lda_mat_rspecifier == "",
                           "You have to specify --mat_lda.");

        return res;
    }

    bool PyKaldi2DecoderConfig::OptionCheck(bool cond, std::string fail_text) {
        if (cond) {
            KALDI_ERR << fail_text;
            return false;
        }
        return true;
    }
}