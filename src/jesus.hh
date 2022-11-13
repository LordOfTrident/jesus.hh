#include <cstdlib> // std::exit, EXIT_FAILURE, EXIT_SUCCESS, size_t, std::malloc, std::free,
#include <cstdio>  // std::fputs, FILE, std::fopen, std::fclose, stdout, stderr, std::fprintf,
                   // std::fputc, std::fflush
#include <cstring> // std::strlen, std::memcpy
#include <cstdint> // std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t,
                   // std::int8_t, std::int16_t, std::int32_t, std::int64_t, std::uintptr_t
#include <cctype>  // std::isdigit

#ifndef JESUS_HH__HEADER_GUARD__
#define JESUS_HH__HEADER_GUARD__

#define EXIT_OK   0
#define EXIT_FAIL 1

#define UNREACHABLE() Jesus::Panic(__FILE__, ":", __LINE__, ": Unreachable")
#define TODO(...)     Jesus::Panic(__FILE__, ":", __LINE__, " TODO: ", __VA_ARGS__)
#define UNUSED(P_VAR) (void)P_VAR

#define ASSERT(P_COND) \
	if (not (P_COND)) \
		Jesus::Panic(__FILE__, ":", __LINE__, ": Assertion '", TO_STR(P_COND), "' failed")

#define SILENCE_RETURN_WARNING() \
	Stderr.Print(__FILE__, ":", __LINE__, ": Reached unreachable return warning silencer\n"); \
	Jesus::Exit(EXIT_FAIL)

#define TO_STR(P_X)   __TO_STR(P_X)
#define __TO_STR(P_X) #P_X

#define ARR_SIZE(P_ARR) (sizeof(P_ARR) / sizeof(P_ARR[0]))

namespace Jesus {
	// TYPES

	using std::size_t;
	using word_t = std::uintptr_t;

	using u8_t  = std::uint8_t ;
	using u16_t = std::uint16_t;
	using u32_t = std::uint32_t;
	using u64_t = std::uint64_t;

	using i8_t  = std::int8_t ;
	using i16_t = std::int16_t;
	using i32_t = std::int32_t;
	using i64_t = std::int64_t;

	// BASIC FUNCTIONS

	template<typename T>
	inline T &&Move(T &p_val) {
		return (T&&)p_val;
	}

	inline void Exit(int p_exitCode) {
		std::exit(p_exitCode);
	}

	template<typename... Args>
	void Panic(Args... p_args);

	// MAYBE CLASS

	template<typename T>
	struct Maybe {
		static inline Maybe None() {
			return Maybe();
		}

		Maybe(): none(true) {}
		Maybe(const T &p_value): none(false), unwrap(p_value) {}
		Maybe(T &&p_value):      none(false), unwrap(p_value) {}

		bool none;
		T    unwrap;

		bool operator ==(const Maybe &p_maybe) {
			return none or p_maybe.none? none == p_maybe.none : unwrap == p_maybe.unwrap;
		}

		bool operator !=(const Maybe &p_maybe) {
			return !(*this == p_maybe);
		}

		bool operator ==(const T &p_val) {
			if (none)
				return false;

			return p_val == unwrap;
		}

		bool operator !=(const T &p_val) {
			return !(*this == p_val);
		}

		template<typename... Args>
		T &UnwrapOrPanic(Args... p_args) {
			if (none)
				Panic(p_args...);

			return unwrap;
		}

		template<typename... Args>
		const T &UnwrapOrPanic(Args... p_args) const {
			if (none)
				Panic(p_args...);
			else
				return unwrap;
		}
	};

	// CONVERSION FUNCTIONS

	Maybe<int> AToI(const char *p_str, Maybe<size_t> p_len = Maybe<size_t>::None());

	// STRINGS

	// TODO: improve the strings class, get rid of repetition, make StringView interact with String
	//       more easily...

	template<typename T>
	class String {
	public:
		String():
			m_len(0),
			m_size(1)
		{
			m_raw = (T*)std::malloc(m_size);
			if (m_raw == nullptr)
				Panic("malloc() fail");

			m_raw[m_len] = '\0';
		}

		String(const T *p_raw, Maybe<size_t> p_len = Maybe<size_t>::None()) {
			ASSERT(p_raw != nullptr);

			m_len  = p_len.none? std::strlen(p_raw) : p_len.unwrap;
			m_size = m_len + 1;

			m_raw = (T*)std::malloc(m_size);
			if (m_raw == nullptr)
				Panic("malloc() fail");

			std::memcpy(m_raw, p_raw, m_len);
			m_raw[m_len] = '\0';
		}

		String(String<T> &&p_str):
			m_len(p_str.m_len),
			m_size(p_str.m_size),
			m_raw(p_str.m_raw)
		{
			p_str.m_raw  = nullptr;
			p_str.m_len  = 0;
			p_str.m_size = 0;
		}

		String(const String<T> &p_str):
			String(p_str.m_raw, p_str.m_len)
		{}

		~String() {
			if (m_raw != nullptr)
				std::free(m_raw);
		}

		String<T> operator +(const String<T> &p_r) {
			ASSERT(m_raw != nullptr and p_r.m_raw != nullptr);

			//void *tmp = realloc(m_raw, newSize);
			//if (tmp == nullptr)
			//	std::free(m_raw);

			size_t newLen  = m_len + p_r.m_len;
			size_t newSize = newLen + 1;

			T *raw = (T*)malloc(newSize);
			if (raw == nullptr)
				Panic("malloc() fail");

			std::memcpy(raw, m_raw, m_len);
			std::memcpy(raw + m_len, p_r.m_raw, p_r.m_len);

			raw[newLen] = '\0';

			return String(raw, newLen, newSize);
		}

		String<T> &operator +=(const String<T> &p_r) {
			ASSERT(m_raw != nullptr and p_r.m_raw != nullptr);

			size_t newLen  = m_len + p_r.m_len;
			size_t newSize = newLen + 1;

			void *tmp = realloc(m_raw, newSize);
			if (tmp == nullptr)
				std::free(m_raw);

			m_raw = (T*)tmp;
			std::memcpy(m_raw + m_len, p_r.m_raw, p_r.m_len);

			m_raw[newLen] = '\0';

			m_len  = newLen;
			m_size = newSize;

			return *this;
		}

		String<T> operator /(const String<T> &p_r) {
			return *this + "/" + p_r;
		}

		String<T> operator /=(const String<T> &p_r) {
			*this += "/" + p_r;
		}

		T operator [](size_t p_idx) {
			return m_raw[p_idx];
		}

		T operator [](int p_idx) {
			return m_raw[p_idx];
		}

		Maybe<size_t> FirstOf(T p_ch, size_t p_idx = 0) {
			ASSERT(p_idx < m_len);

			for (size_t i = p_idx; i < m_len; ++ i) {
				if (m_raw[i] == p_ch)
					return i;
			}

			return Maybe<size_t>::None();
		}

		Maybe<size_t> FirstOf(String<T> p_str, size_t p_idx = 0) {
			ASSERT(p_idx + p_str.Length() <= m_len);

			for (size_t i = p_idx; i < m_len; ++ i) {
				bool   matches = true;
				size_t j       = 0;
				for (; j < p_str.m_len; ++ j) {
					if (j + i >= m_len)
						return Maybe<size_t>::None();

					if (m_raw[j + i] != p_str[j]) {
						matches = false;

						break;
					}
				}

				if (matches)
					return i;
			}

			return Maybe<size_t>::None();
		}

		String<T> operator %(const String<T> &p_r) {
			Maybe<size_t> idx = FirstOf("%v");
			ASSERT(not idx.none);

			String formatted;
			if (idx.unwrap != 0)
				formatted += Sub(0, idx.unwrap);

			formatted += p_r;

			if (idx.unwrap + 2 < m_len)
				formatted += Sub(idx.unwrap + 2);

			return Move(formatted);
		}

		Maybe<int> ToInt(int p_base = 10) {
			UNUSED(p_base);

			return AToI(m_raw, m_len);
		}

		size_t Length() const {
			return m_len;
		}

		const T *Raw() const {
			return m_raw;
		}

		bool Prefix(const String<T> &p_prefix) const {
			if (p_prefix.m_len > m_len)
				return false;

			for (size_t i = 0; i < p_prefix.m_len; ++ i) {
				if (m_raw[i] != p_prefix.m_raw[i])
					return false;
			}

			return true;
		}

		bool Suffix(const String<T> &p_suffix) const {
			if (p_suffix.m_len > m_len)
				return false;

			for (size_t i = 0; i < p_suffix.m_len; ++ i) {
				if (m_raw[m_len - i - 1] != p_suffix.m_raw[p_suffix.m_len - i - 1])
					return false;
			}

			return true;
		}

		size_t Occurs(T p_ch) {
			size_t count = 0;

			for (size_t i = 0; i < m_len; ++ i) {
				if (m_raw[i] == p_ch)
					++ count;
			}

			return count;
		}

		String<T> Sub(size_t p_start, Maybe<size_t> p_count = Maybe<size_t>::None()) {
			return String<T>(m_raw + p_start,
			                 p_count.none? m_len - p_start : p_count.unwrap);
		}

	private:
		String(T *p_raw, size_t p_len, size_t p_size):
			m_len(p_len),
			m_size(p_size),
			m_raw(p_raw)
		{}

		size_t m_len, m_size;
		T     *m_raw;
	};

	template<typename T>
	class StringView {
	public:
		StringView(const T *p_raw, Maybe<size_t> p_len = Maybe<size_t>::None()):
			m_raw(p_raw)
		{
			ASSERT(m_raw != nullptr);

			m_len = p_len.none? std::strlen(p_raw) : p_len.unwrap;
		}

		Maybe<size_t> FirstOf(T p_ch, size_t p_idx = 0) {
			ASSERT(p_idx < m_len);

			for (size_t i = p_idx; i < m_len; ++ i) {
				if (m_raw[i] == p_ch)
					return i;
			}

			return Maybe<size_t>::None();
		}

		Maybe<size_t> FirstOf(String<T> p_str, size_t p_idx = 0) {
			ASSERT(p_idx + p_str.Length() <= m_len);

			for (size_t i = p_idx; i < m_len; ++ i) {
				bool   matches = true;
				size_t j       = 0;
				for (; j < p_str.Length(); ++ j) {
					if (j + i >= m_len)
						return Maybe<size_t>::None();

					if (m_raw[j + i] != p_str[j]) {
						matches = false;

						break;
					}
				}

				if (matches)
					return i;
			}

			return Maybe<size_t>::None();
		}

		String<T> operator %(const String<T> &p_r) {
			Maybe<size_t> idx = FirstOf("%v");
			ASSERT(not idx.none);

			// TODO: make .Raw() not be required here
			String<T> formatted;
			if (idx.unwrap != 0)
				formatted += String(Sub(0, idx.unwrap).Raw());

			formatted += p_r;

			// TODO: make .Raw() not be required here
			if (idx.unwrap + 2 < m_len)
				formatted += Sub(idx.unwrap + 2).Raw();

			return Move(formatted);
		}

		Maybe<int> ToInt(int p_base = 10) {
			UNUSED(p_base);

			return AToI(m_raw, m_len);
		}

		size_t Length() const {
			return m_len;
		}

		const T *Raw() const {
			return m_raw;
		}

		bool operator ==(const StringView<T> &p_b) const {
			if (m_len == p_b.m_len) {
				for (size_t i = 0; i < m_len; ++ i) {
					if (m_raw[i] != p_b.m_raw[i])
						return false;
				}

				return true;
			} else
				return false;
		}

		bool operator !=(const StringView<T> &p_b) const {
			return !(*this == p_b);
		}

		bool Prefix(const StringView<T> &p_prefix) const {
			if (p_prefix.m_len > m_len)
				return false;

			for (size_t i = 0; i < p_prefix.m_len; ++ i) {
				if (m_raw[i] != p_prefix.m_raw[i])
					return false;
			}

			return true;
		}

		bool Suffix(const StringView<T> &p_suffix) const {
			if (p_suffix.m_len > m_len)
				return false;

			for (size_t i = 0; i < p_suffix.m_len; ++ i) {
				if (m_raw[m_len - i - 1] != p_suffix.m_raw[p_suffix.m_len - i - 1])
					return false;
			}

			return true;
		}

		size_t Occurs(T p_ch) {
			size_t count = 0;

			for (size_t i = 0; i < m_len; ++ i) {
				if (m_raw[i] == p_ch)
					++ count;
			}

			return count;
		}

		StringView<T> Sub(size_t p_start, Maybe<size_t> p_count = Maybe<size_t>::None()) {
			return StringView<T>(m_raw + p_start,
			                     p_count.none? m_len - p_start : p_count.unwrap);
		}

	private:
		size_t   m_len;
		const T *m_raw;
	};

	StringView<char> operator ""_sv(const char *p_raw, size_t p_len) {
		return StringView(p_raw, p_len);
	}

	String<char> operator ""_s(const char *p_raw, size_t p_len) {
		return String(p_raw, p_len);
	}

	// ERROR SYSTEM

	struct Error {
		static inline Error Fine() {
			return Error();
		}

		template<typename... Args>
		static inline Error Make(Args... p_args) {
			String<char> str;

			((str += ("%v"_sv % p_args)), ...);

			return Error(str.Raw());
		}

		StringView<char> Desc() const {
			return m_desc.Raw();
		}

		bool Ok() const {
			return m_ok;
		}

	private:
		Error():                               m_ok(true),  m_desc("")     {}
		Error(const StringView<char> &p_desc): m_ok(false), m_desc(p_desc.Raw()) {}

		bool         m_ok;
		String<char> m_desc;
	};

	inline void Panic(const Error &p_err) {
		Panic(p_err.Desc());
	}

	// TODO: implement ErrorOr (requires a Variant system)

	// STREAMS

	class Stream {
	public:
		// TODO: use ErrorOr to return ErrorOr<Stream> (Maybe<Stream> is not possible because
		//       Stream does not have a default constructor)
		static Stream OpenFile(const StringView<char> &p_path,
		                       const StringView<char> &p_flags);

		Stream(FILE *p_file);

	protected:
		FILE *m_file;
	};

	// OUTPUT STREAMS

	class OutputStream : public Stream {
	public:
		static OutputStream OpenFile(const StringView<char> &p_path,
		                             bool p_binary);

		OutputStream(FILE *p_file);
	};

	class TextOutputStream : public OutputStream {
	public:
		static TextOutputStream OpenFile(const StringView<char> &p_path);

		TextOutputStream(FILE *p_file);

		void Write(bool        p_bool);
		void Write(char        p_ch);
		void Write(int         p_int, int p_base = 10);
		void Write(size_t      p_size);
		void Write(const char *p_raw);

		template<typename T>
		void Write(const StringView<T> &p_sv) {
			for (size_t i = 0; i < p_sv.Length(); ++ i) {
				Write(p_sv.Raw()[i]);
			}
		}

		template<typename T>
		void Write(const String<T> &p_sv) {
			for (size_t i = 0; i < p_sv.Length(); ++ i) {
				Write(p_sv.Raw()[i]);
			}
		}

		template<typename... Args>
		void Print(Args... p_args) {
			(Write(p_args), ...);
		}

		template<typename... Args>
		void PrintLn(Args... p_args) {
			(Write(p_args), ...);
			Write('\n');
			Flush();
		}

		void Flush();
	};

	extern TextOutputStream Stdout;
	extern TextOutputStream Stderr;

	template<typename... Args>
	void Panic(Args... p_args) {
		Stderr.Write("Panic: ");
		(Stderr.Write(p_args), ...);
		Stderr.Write('\n');

		Exit(EXIT_FAIL);
	}
}

#endif

#ifdef JESUS_HH__IMPLEMENTATION__

namespace Jesus {
	Maybe<int> AToI(const char *p_str, Maybe<size_t> p_len) {
		size_t len = p_len.none? std::strlen(p_str) : p_len.unwrap;

		int num = 0;
		for (size_t i = 0; i < len; ++ i) {
			if (not std::isdigit(p_str[i]))
				return Maybe<int>::None();

			num = num * 10 + (p_str[i] - '0');
		}

		return num;
	}

	TextOutputStream Stdout(::stdout);
	TextOutputStream Stderr(::stderr);

	Stream Stream::OpenFile(const StringView<char> &p_path,
	                               const StringView<char> &p_flags) {
		FILE *file = fopen(p_path.Raw(), p_flags.Raw());
		if (file == nullptr)
			return Stream(file);

		return Stream(file);
	}

	Stream::Stream(FILE *p_file):
		m_file(p_file)
	{
		ASSERT(m_file != nullptr);
	}

	OutputStream OutputStream::OpenFile(const StringView<char> &p_path, bool p_binary) {
		FILE *file = fopen(p_path.Raw(), p_binary? "wb" : "w");
		if (file == nullptr)
			return OutputStream(file);

		return OutputStream(file);
	}

	OutputStream::OutputStream(FILE *p_file):
		Stream(p_file)
	{}

	TextOutputStream TextOutputStream::OpenFile(const StringView<char> &p_path) {
		FILE *file = fopen(p_path.Raw(), "w");
		if (file == nullptr)
			return TextOutputStream(file);

		return TextOutputStream(file);
	}

	TextOutputStream::TextOutputStream(FILE *p_file):
		OutputStream(p_file)
	{}

	void TextOutputStream::Write(bool p_bool) {
		std::fprintf(m_file, p_bool? "true" : "false");
	}

	void TextOutputStream::Write(char p_ch) {
		std::fputc(p_ch, m_file);
	}

	void TextOutputStream::Write(int p_int, int p_base) {
		switch (p_base) {
		case 8:  std::fprintf(m_file, "%o", p_int); break;
		case 16: std::fprintf(m_file, "%x", p_int); break;

		default: std::fprintf(m_file, "%d", p_int);
		}
	}

	void TextOutputStream::Write(size_t p_size) {
		std::fprintf(m_file, "%zu", p_size);
	}

	void TextOutputStream::Write(const char *p_raw) {
		std::fputs(p_raw, m_file);
	}

	void TextOutputStream::Flush() {
		std::fflush(m_file);
	}
}

#endif
