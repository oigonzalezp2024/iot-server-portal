<?php
/**
 * Script para simular un cliente externo (ESP8266) usando llamadas HTTP reales (cURL).
 */

date_default_timezone_set('America/Bogota'); 

// --- CONFIGURACIÓN DE PRUEBA (Debe coincidir con tu .env) ---
$API_URL = 'http://localhost/api/dispositivo_nuevo/'; 
$API_KEY_CORRECTA = 'tu_clave_secreta_api_aqui'; 
$logFile = '../logs/ip_log_quemados.log'; // Asegúrate de que esta ruta es correcta desde la raíz de tu proyecto

// La función logMessage DEBE estar aquí para que el tester registre sus acciones
if (!function_exists('logMessage')) {
    function logMessage($message) {
        global $logFile;
        $timestamp = date('Y-m-d H:i:s');
        file_put_contents($logFile, "[$timestamp] $message\n", FILE_APPEND | LOCK_EX);
    }
}

$tests_config = [
    [
        'name' => 'Test 1: Inserción Inicial (INSERT)',
        'headers' => ['X-Device-Auth: ' . $API_KEY_CORRECTA],
        'post_data' => ['device_id' => 'ESP_TEST_012', 'ip_address' => '10.0.0.112'], 
        'expected_status' => 200 
    ],
    [
        'name' => 'Test 2: Actualización (UPSERT)',
        'headers' => ['X-Device-Auth: ' . $API_KEY_CORRECTA],
        'post_data' => ['device_id' => 'ESP_TEST_012', 'ip_address' => '10.0.0.116'],
        'expected_status' => 200
    ],
    [
        'name' => 'Test 3: Fallo de Autenticación (401)',
        'headers' => ['X-Device-Auth: CLAVE_INCORRECTA'],
        'post_data' => ['device_id' => 'HACKER_1', 'ip_address' => '99.99.99.99'],
        'expected_status' => 401
    ],
    [
        'name' => 'Test 4: Fallo de Validación (Falta IP)',
        'headers' => ['X-Device-Auth: ' . $API_KEY_CORRECTA],
        'post_data' => ['device_id' => 'ESP_TEST_013'],
        'expected_status' => 400
    ]
];

$results = [];

// --- FUNCIÓN DE EJECUCIÓN cURL ---
function executeCurlTest($url, $data, $headers) {
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode($data));
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_HTTPHEADER, array_merge($headers, ['Content-Type: application/json']));
    
    $response = curl_exec($ch);
    $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    curl_close($ch);
    
    return ['output' => $response, 'http_code' => $http_code];
}

// --- EJECUCIÓN DE PRUEBAS ---
foreach ($tests_config as $index => $test) {
    
    $test_id = $index + 1;
    logMessage("--- INICIO TEST $test_id: {$test['name']} (Llamada Externa) ---"); 
    
    $curl_result = executeCurlTest($API_URL, $test['post_data'], $test['headers']);
    
    $output = $curl_result['output'];
    $http_code = $curl_result['http_code'];
    
    $decoded_output = json_decode($output, true);

    if ($decoded_output === null && !empty($output)) {
         $test_status_message = "❌ FAILED: Error en la respuesta HTTP o Salida no JSON. Salida: " . substr(strip_tags($output), 0, 100);
         $test_passed = false;
         $http_code = $http_code > 0 ? $http_code : 500;
    } else {
        $test_passed = ($http_code === $test['expected_status']);
        
        if ($test_passed) {
            $test_status_message = "✅ PASSED: HTTP {$http_code}. Acción: " . ($decoded_output['action'] ?? 'Validación/Error');
        } else {
            $endpoint_error_message = $decoded_output['message'] ?? 'Respuesta JSON inválida o vacía.';
            $test_status_message = "❌ FAILED: Esperaba HTTP {$test['expected_status']}, se obtuvo HTTP {$http_code}. Mensaje del Endpoint: \"{$endpoint_error_message}\"";
        }
    }
    
    logMessage("--- FIN TEST $test_id: {$test['name']} - Resultado: " . ($test_passed ? 'PASSED' : 'FAILED') . " (HTTP {$http_code}) ---"); 

    $results[] = [
        'test_name' => $test['name'],
        'result' => $test_status_message, 
        'expected_status' => $test['expected_status'],
        'http_status' => $http_code,
        'response_json' => $decoded_output,
        'overall_success' => $test_passed
    ];
}

header('Content-Type: application/json');
http_response_code(200); 
echo json_encode($results, JSON_PRETTY_PRINT);
