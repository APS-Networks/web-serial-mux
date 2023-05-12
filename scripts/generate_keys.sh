#! /usr/bin/env bash

set -e

script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

cd ${script_dir}

ca_csr_template=${script_dir}/ca.csr_template.ini.in
srv_csr_template=${script_dir}/srv.csr_template.ini.in
srv_ext_template=${script_dir}/srv.ext_template.ini.in

sudo apt-get install jq ca-certificates python3-venv
python3 -m venv .venv
. .venv/bin/activate

pip install j2cli




# Certificate Authority
# =====================
ca_csr_json=$(mktemp)
jq '.ca' ${script_dir}/cert_config.json > ${ca_csr_json}
ca_common_name=$(jq '."common_name"' ${ca_csr_json} | cut -d '"' -f 2)
ca_csr_config=${script_dir}/${ca_common_name}.csr.ini
ca_dh_param=${script_dir}/${ca_common_name}.dh.pem
ca_private_key=${script_dir}/${ca_common_name}.key.pem
ca_certificate=${script_dir}/${ca_common_name}.crt

if [[ ! -f ${ca_certificate} ]]
then

    echo "CA CSR jSON:    ${ca_csr_json}"
    echo "CA Common Name: ${ca_common_name}"
    echo "CA CSR Config:  ${ca_csr_config}"
    echo "CA DH Param:    ${ca_dh_param}"
    echo "CA Private Key: ${ca_private_key}"
    echo "CA Certificate: ${ca_certificate}"

    # CA Private Key
    # NOTE: Not password protected. Provide `-des3` parameter
    echo "[Info ] Generating CA private key"
    openssl genrsa -out ${ca_private_key} 2048

    # # CA CSR
    echo "[Info ] Creating CA CSR config"
    j2 -f json ${ca_csr_template} ${ca_csr_json} > ${ca_csr_config}

    echo "[Info ] Signing CA key"
    openssl req -x509 -new -nodes -key ${ca_private_key} \
        -sha256 -days 1024 -out ${ca_certificate} \
        -config ${ca_csr_config}

    echo "[Info ] CA Certificate Contents:"
    openssl x509 -noout -in ${ca_certificate} -text

fi
# Server Certificate
# ==================
srv_csr_json=$(mktemp)
jq '.server' ${script_dir}/cert_config.json > ${srv_csr_json}
srv_common_name=$(jq '."common_name"' ${srv_csr_json} | cut -d '"' -f 2)
srv_csr_config=${script_dir}/${srv_common_name}.csr.ini
srv_dh_param=${script_dir}/${srv_common_name}.dh.pem
srv_private_key=${script_dir}/${srv_common_name}.key.pem
srv_certificate=${script_dir}/${srv_common_name}.crt
srv_ext_config=${script_dir}/${srv_common_name}.ext.ini


echo "Server CSR jSON:    ${srv_csr_json}"
echo "Server Common Name: ${srv_common_name}"
echo "Server CSR Config:  ${srv_csr_config}"
echo "Server DH Param:    ${srv_dh_param}"
echo "Server Private Key: ${srv_private_key}"
echo "Server Certificate: ${srv_certificate}"


echo "[Info ] Generating server private key"
openssl genrsa -out ${srv_private_key} 2048

echo "[Info ] Creating server CSR config"
cat ${srv_csr_json} | j2 -f json ${srv_csr_template} > ${srv_csr_config}
cat ${srv_csr_json} | j2 -f json ${srv_ext_template} > ${srv_ext_config}

echo "[Info ] Creating server CSR"
openssl req -new -sha256 \
    -key ${srv_private_key} \
    -config ${srv_csr_config} \
    -out ${srv_common_name}.csr

echo "[Info ] Server CSR Contents:"
openssl req -noout -in ${srv_common_name}.csr -text
# read

openssl x509 -req -in ${srv_common_name}.csr \
    -CA ${ca_certificate} \
    -CAkey ${ca_private_key} \
    -CAcreateserial \
    -out ${srv_certificate} \
    -days 500 \
    -extfile ${srv_ext_config} \
    -sha256

echo "[Info ] Server Certificate Contents:"
openssl x509 -noout -in ${srv_certificate} -text

# read

# Diffie-Helman Parameters
if [ ! -f ${srv_dh_param} ]
then
    openssl dhparam -out ${srv_dh_param} 2048
fi

