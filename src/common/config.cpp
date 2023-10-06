#include "config.h"

void c_config::create(const std::filesystem::path& filepath, s_blur_settings current_settings) {
	std::ofstream output(filepath);

	output << "[blur v" << blur.BLUR_VERSION << "]" << "\n";

	output << "- blur" << "\n";
	output << "blur: " << (current_settings.blur ? "true" : "false") << "\n";
	output << "blur amount: " << current_settings.blur_amount << "\n";
	output << "blur output fps: " << current_settings.blur_output_fps << "\n";
	output << "blur weighting: " << current_settings.blur_weighting << "\n";

	output << "\n";
	output << "- interpolation" << "\n";
	output << "interpolate: " << (current_settings.interpolate ? "true" : "false") << "\n";
	output << "interpolated fps: " << current_settings.interpolated_fps << "\n";

	output << "\n";
	output << "- rendering" << "\n";
	output << "quality: " << current_settings.quality << "\n";
	output << "deduplicate: " << (current_settings.deduplicate ? "true" : "false") << "\n";
	output << "preview: " << (current_settings.preview ? "true" : "false") << "\n";
	output << "detailed filenames: " << (current_settings.detailed_filenames ? "true" : "false") << "\n";

	output << "\n";
	output << "- timescale" << "\n";
	output << "input timescale: " << current_settings.input_timescale << "\n";
	output << "output timescale: " << current_settings.output_timescale << "\n";
	output << "adjust timescaled audio pitch: " << (current_settings.output_timescale_audio_pitch ? "true" : "false") << "\n";

	output << "\n";
	output << "- filters" << "\n";
	output << "brightness: " << current_settings.brightness << "\n";
	output << "saturation: " << current_settings.saturation << "\n";
	output << "contrast: " << current_settings.contrast << "\n";

	output << "\n";
	output << "- advanced rendering" << "\n";
	output << "gpu interpolation: " << (current_settings.gpu_interpolation ? "true" : "false") << "\n";
	output << "gpu rendering: " << (current_settings.gpu_rendering ? "true" : "false") << "\n";
	output << "gpu type (nvidia/amd/intel): " << current_settings.gpu_type << "\n";
	output << "video container: " << current_settings.video_container << "\n";
	output << "custom ffmpeg filters: " << current_settings.ffmpeg_override << "\n";
	output << "debug: " << (current_settings.debug ? "true" : "false") << "\n";

	output << "\n";
	output << "- advanced blur" << "\n";
	output << "blur weighting gaussian std dev: " << current_settings.blur_weighting_gaussian_std_dev << "\n";
	output << "blur weighting triangle reverse: " << (current_settings.blur_weighting_triangle_reverse ? "true" : "false") << "\n";
	output << "blur weighting bound: " << current_settings.blur_weighting_bound << "\n";

	output << "\n";
	output << "- advanced interpolation" << "\n";
	// output << "interpolation program (svp/rife/rife-ncnn): " << current_settings.interpolation_program << "\n";
	output << "interpolation preset: " << current_settings.interpolation_preset << "\n";
	output << "interpolation algorithm: " << current_settings.interpolation_algorithm << "\n";
	output << "interpolation block size: " << current_settings.interpolation_blocksize << "\n";
	output << "interpolation speed: " << current_settings.interpolation_speed << "\n";
	output << "interpolation mask area: " << current_settings.interpolation_mask_area << "\n";
	
	if (current_settings.manual_svp) {
		output << "\n";
		output << "- manual svp override" << "\n";
		output << "manual svp: " << (current_settings.manual_svp ? "true" : "false") << "\n";
		output << "super string: " << current_settings.super_string << "\n";
		output << "vectors string: " << current_settings.vectors_string << "\n";
		output << "smooth string: " << current_settings.smooth_string << "\n";
	}
}

std::filesystem::path c_config::get_config_filename(const std::filesystem::path& video_folder) {
	return video_folder / filename;
}

s_blur_settings c_config::parse(const std::filesystem::path& config_filepath) {
	auto read_config = [&]() {
		std::map<std::string, std::string> config = {};

		// retrieve all of the variables in the config file
		std::ifstream input(config_filepath);
		std::string line;
		while (std::getline(input, line)) {
			// get key & value
			auto pos = line.find(':');
			if (pos == std::string::npos) // not a variable
				continue;

			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);

			// trim whitespace
			key = helpers::trim(key);
			if (key == "")
				continue;

			value = helpers::trim(value);

			if (key != "custom ffmpeg filters") {
				// remove all spaces in values (it breaks stringstream string parsing, this is a dumb workaround) todo: better solution
				std::string::iterator end_pos = std::remove(value.begin(), value.end(), ' ');
				value.erase(end_pos, value.end());
			}

			config[key] = value;
		}

		return config;
	};

	auto config = read_config();

	auto config_get = [&]<typename t>(const std::string & var, t & out) {
		if (!config.contains(var)) {
			helpers::debug_log(fmt::format("config missing variable '{}'", var));
			return;
		}

		try {
			std::stringstream ss(config[var]);
			ss.exceptions(std::ios::failbit); // enable exceptions
			ss >> std::boolalpha >> out; // boolalpha: enable true/false bool parsing
		}
		catch (const std::exception&) {
			helpers::debug_log("failed to parse config variable '{}' (value: {})", var, config[var]);
			return;
		}
	};

	auto config_get_str = [&](const std::string& var, std::string& out) { // todo: clean this up i cant be bothered rn
		if (!config.contains(var)) {
			helpers::debug_log(fmt::format("config missing variable '{}'", var));
			return;
		}

		out = config[var];
	};

	s_blur_settings settings;

	config_get("blur", settings.blur);
	config_get("blur amount", settings.blur_amount);
	config_get("blur output fps", settings.blur_output_fps);
	config_get_str("blur weighting", settings.blur_weighting);

	config_get("interpolate", settings.interpolate);
	config_get_str("interpolated fps", settings.interpolated_fps);

	config_get("brightness", settings.brightness);
	config_get("saturation", settings.saturation);
	config_get("contrast", settings.contrast);

	config_get("quality", settings.quality);
	config_get("deduplicate", settings.deduplicate);
	config_get("preview", settings.preview);
	config_get("detailed filenames", settings.detailed_filenames);

	config_get("input timescale", settings.input_timescale);
	config_get("output timescale", settings.output_timescale);
	config_get("adjust timescaled audio pitch", settings.output_timescale_audio_pitch);

	config_get("gpu interpolation", settings.gpu_interpolation);
	config_get("gpu rendering", settings.gpu_rendering);
	config_get_str("gpu type (nvidia/amd/intel)", settings.gpu_type);
	config_get("video container", settings.video_container);
	config_get_str("custom ffmpeg filters", settings.ffmpeg_override);
	config_get("debug", settings.debug);

	config_get("blur weighting gaussian std dev", settings.blur_weighting_gaussian_std_dev);
	config_get("blur weighting triangle reverse", settings.blur_weighting_triangle_reverse);
	config_get_str("blur weighting bound", settings.blur_weighting_bound);

	// config_get_str("interpolation program (svp/rife/rife-ncnn)", settings.interpolation_program);
	config_get_str("interpolation preset", settings.interpolation_preset);
	config_get("interpolation algorithm", settings.interpolation_algorithm);
	config_get("interpolation block size", settings.interpolation_blocksize);
	config_get_str("interpolation speed", settings.interpolation_speed);
	config_get("interpolation mask area", settings.interpolation_mask_area);

	config_get("manual svp", settings.manual_svp);
	config_get_str("super string", settings.super_string);
	config_get_str("vectors string", settings.vectors_string);
	config_get_str("smooth string", settings.smooth_string);

	// recreate the config file using the parsed values (keeps nice formatting)
	create(config_filepath, settings);

	return settings;
}

s_blur_settings c_config::get_config(const std::filesystem::path& config_filepath, bool use_global) {
	bool local_cfg_exists = std::filesystem::exists(config_filepath);

	auto global_cfg_path = blur.path / filename;
	bool global_cfg_exists = std::filesystem::exists(global_cfg_path);

	std::filesystem::path cfg_path;
	if (use_global && !local_cfg_exists && global_cfg_exists) {
		cfg_path = global_cfg_path;

		if (blur.verbose)
			printf("Using global config\n");
	}
	else {
		// check if the config file exists, if not, write the default values
		if (!local_cfg_exists) {
			config.create(config_filepath);

			wprintf(L"Configuration file not found, default config generated at %s\n", config_filepath.wstring().c_str());
		}

		cfg_path = config_filepath;
	}

	return parse(cfg_path);
}

nlohmann::json s_blur_settings::to_json() {
	nlohmann::json j;

	j["blur"] = this->blur;
	j["blur_amount"] = this->blur_amount;
	j["blur_output_fps"] = this->blur_output_fps;
	j["blur_weighting"] = this->blur_weighting;

	j["interpolate"] = this->interpolate;
	j["interpolated_fps"] = this->interpolated_fps;

	j["input_timescale"] = this->input_timescale;
	j["output_timescale"] = this->output_timescale;
	j["output_timescale_audio_pitch"] = this->output_timescale_audio_pitch;

	j["brightness"] = this->brightness;
	j["saturation"] = this->saturation;
	j["contrast"] = this->contrast;

	j["quality"] = this->quality;
	j["deduplicate"] = this->deduplicate;
	j["preview"] = this->preview;
	j["detailed_filenames"] = this->detailed_filenames;

	j["gpu_interpolation"] = this->gpu_interpolation;
	j["gpu_rendering"] = this->gpu_rendering;
	j["gpu_type"] = this->gpu_type;
	j["video_container"] = this->video_container;
	j["ffmpeg_override"] = this->ffmpeg_override;
	j["debug"] = this->debug;

	j["blur_weighting_gaussian_std_dev"] = this->blur_weighting_gaussian_std_dev;
	j["blur_weighting_triangle_reverse"] = this->blur_weighting_triangle_reverse;
	j["blur_weighting_bound"] = this->blur_weighting_bound;

	j["interpolation_program"] = this->interpolation_program;
	j["interpolation_preset"] = this->interpolation_preset;
	j["interpolation_algorithm"] = this->interpolation_algorithm;
	j["interpolation_blocksize"] = this->interpolation_blocksize;
	j["interpolation_speed"] = this->interpolation_speed;
	j["interpolation_mask_area"] = this->interpolation_mask_area;

	j["manual_svp"] = this->manual_svp;
	j["super_string"] = this->super_string;
	j["vectors_string"] = this->vectors_string;
	j["smooth_string"] = this->smooth_string;

	return j;
}
