//
// Created by Богдан Василенко on 21/12/2019.
//

#ifndef AFFINE_CIPHERS_PROGRAM_H
#define AFFINE_CIPHERS_PROGRAM_H

#include <string>
#include <utility>
#include <cstdint>
#include <vector>
#include <functional>
#include <unordered_map>


namespace affine_ciphers_ns {

    struct key {
        std::uint8_t a;
        std::uint8_t b;

        key() = default;
        key(std::uint8_t i_a, std::uint8_t i_b)
                : a(i_a), b(i_b) {}

        bool operator==(const key &i_other) const {
            return a == i_other.a && b == i_other.b;
        }

        std::string to_string() const;
    };

    class program {
    public:
        struct settings {
            enum text_lang_t { Eng = 1, Rus } text_lang = text_lang_t::Eng;
            enum non_dict_rule_t { Ignore = 1, Keep } non_dict_rule = non_dict_rule_t::Keep;
            enum upper_lower_rule_t { Mix = 1, Only_upper, Only_lower} upper_lower_rule = upper_lower_rule_t::Mix;

            std::string to_string() const;
        };

        struct hack_res
        {
            double standard_deviation = 0.;
            std::string decrypted_str;
            key hacked_key;
        };

    public:
        template<typename STR_T>
        std::pair<STR_T, key> encrypt(const STR_T& i_str) const;

        template<typename STR_T>
        STR_T decrypt(const STR_T& i_str, key i_key) const;

        std::vector<hack_res> hack(const std::string& i_str) const;

        void set_settings(const settings& i_settings) { m_settings = i_settings; }
        const settings& get_settings() const { return m_settings; }

    private:
        key gen_key() const;

        using translate_fn = std::function<size_t (size_t)>;
        std::string translate_msg(const std::string& i_str, translate_fn i_translate_fn) const;
        std::wstring translate_msg(const std::wstring& i_str, translate_fn i_translate_fn) const;

        std::pair<const std::wstring&, std::size_t> find_ch_in_dict(wchar_t i_ch) const;

        std::unordered_map<wchar_t, double> analyze_freqs(const std::wstring& i_str) const;

    private:
        settings m_settings;

        const static std::wstring eng_dict;
        const static std::wstring eng_upper_dict;
        const static std::vector<double> eng_stats;
        const static std::vector<std::uint8_t> possible_a_eng;

        const static std::wstring rus_dict;
        const static std::wstring rus_upper_dict;
        const static std::vector<double> rus_stats;
        const static std::vector<std::uint8_t> possible_a_rus;
    };

    template<typename STR_T>
    std::pair<STR_T, key> program::encrypt(const STR_T& i_str) const
    {
        const auto enc_key = gen_key();

        const std::size_t dict_size = (m_settings.text_lang == settings::Eng ? eng_dict.size() : rus_dict.size());
        auto enc_fn = [&enc_key, dict_size](std::size_t i_curr_pos)
        {
            return (i_curr_pos * enc_key.a + enc_key.b) % dict_size;
        };

        return { translate_msg(i_str, enc_fn), enc_key };
    };

    template<typename STR_T>
    STR_T program::decrypt(const STR_T& i_str, key i_key) const
    {
        const std::size_t dict_size = (m_settings.text_lang == settings::Eng ? eng_dict.size() : rus_dict.size());
        std::uint8_t a_inv = i_key.a % dict_size;
        for(std::uint8_t x = 1; x < dict_size; ++x)
        {
            if((a_inv * x) % dict_size == 1)
            {
                a_inv = x;
                break;
            }
        }

        auto dec_fn = [&i_key, a_inv, dict_size](std::size_t i_curr_pos)
        {
            return (a_inv * (i_curr_pos + dict_size - i_key.b)) % dict_size;
        };

        return translate_msg(i_str, dec_fn);
    }
}

#endif //AFFINE_CIPHERS_PROGRAM_H
