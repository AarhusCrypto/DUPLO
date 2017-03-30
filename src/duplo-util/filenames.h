#ifndef DUPLO_UTIL_FILENAMES_H_
#define DUPLO_UTIL_FILENAMES_H_

static std::string STORAGE_PREFIX("storage/");

static std::string SENDER_BASE_OT(STORAGE_PREFIX + "sender_base_ot");
static std::string RECEIVER_BASE_OT(STORAGE_PREFIX + "receiver_base_ot");
static std::string CONST_INP_BUCKET_NAME("const_inp_bucket_circuit");
static std::string EVAL_INP_BUCKET_NAME("eval_inp_bucket_circuit");

static std::string TABLES_SUFFIX("_tables");
static std::string SOLDERING_SUFFIX("_solderings");
static std::string AUXDATA_SUFFIX("_auxdata");

static std::string VERTICAL_SOLDERINGS_SUFFIX("_vertical_solderings");

static std::string AUTHS_INPUT_PREFIX("auths_input");
static std::string AUTHS_SUFFIX("_auths");
static std::string AUTHS_SOLDERINGS_SUFFIX("_auths_solderings");
static std::string AUTHS_DELTA_SOLDERINGS_SUFFIX("_auths_delta_solderings");
static std::string AUTHS_IDS_SUFFIX("_auths_ids");

static std::string CONST_RECOV_PREFIX("const_recov");
static std::string EVAL_RECOV_PREFIX("eval_recov");
static std::string RECOV_SUFFIX("_recov");
static std::string RECOV_DATA_SUFFIX("_recov_data");

static std::string CONST_PREPROCESS_PREFIX("const_preprocess");
static std::string EVAL_PREPROCESS_PREFIX("eval_preprocess");

static std::string CONST_INP_BUCKET_PREFIX("const_inp_bucket");
static std::string EVAL_INP_BUCKET_PREFIX("eval_inp_bucket");

static std::string INPUT_MASKS_AUXDATA_SUFFIX("_input_masks_auxdata");
static std::string INPUT_MASKS_DELTA_AUXDATA_SUFFIX("_input_masks_delta_auxdata");
static std::string INPUT_MASKS_CORRECTIONS_SUFFIX("_input_masks_corrections");
static std::string INPUT_MASKS_DELTA_CORRECTION_SUFFIX("_input_masks_delta_correction");

static std::string OUTPUT_MASKS_AUXDATA_SUFFIX("_output_masks_auxdata");

static std::string INPUT_PERM_BITS_SUFFIX("_input_perm_bits");

#endif /* DUPLO_UTIL_FILENAMES_H_ */