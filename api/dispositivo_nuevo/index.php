<?php
require __DIR__ . '/../vendor/autoload.php';

// Carga de Dotenv - CONFIRMADO: El .env está en el directorio padre (../)
$dotenv = Dotenv\Dotenv::createImmutable(__DIR__."/../");
$dotenv->safeLoad();

// --- CARGA DE VARIABLES: Uso de $_ENV['CLAVE'] ?? '' para evitar Undefined Index Errors ---
$timezoneId =     $_ENV['APP_TIMEZONE'] ?? '';
$dbHost =         $_ENV['DB_HOST'] ?? '';
$dbName =         $_ENV['DB_NAME'] ?? '';
$dbUser =         $_ENV['DB_USER'] ?? '';
$dbPass =         $_ENV['DB_PASS'] ?? '';
$tableName =      $_ENV['DB_TABLE_NAME'] ?? '';
$expectedApiKey = $_ENV['EXPECTED_API_KEY'] ?? '';
$apiKeyHeader =   $_ENV['API_KEY_HEADER'] ?? 'X-Device-Auth';
$logFile =        $_ENV['LOG_FILE_PATH'] ?? '';

// --- Carga de variables de control (sin quemar) ---
$expectedMethod = $_ENV['API_EXPECTED_METHOD'] ?? 'POST';
$requiredFieldsStr = $_ENV['API_REQUIRED_FIELDS'] ?? 'device_id,ip_address';
$contentType =    $_ENV['API_RESPONSE_CONTENT_TYPE'] ?? 'application/json';


// --- CONFIGURACIÓN Y VARIABLES FINALES ---
date_default_timezone_set($timezoneId); 
$requiredFields = array_map('trim', explode(',', $requiredFieldsStr));

if (!function_exists('logMessage')) {
    function logMessage($message) {
        global $logFile;
        if (empty($logFile)) { return; }
        $timestamp = date('Y-m-d H:i:s');
        file_put_contents($logFile, "[$timestamp] $message\n", FILE_APPEND | LOCK_EX);
    }
}

function sendResponse($data, $httpCode = 200) {
    global $contentType;
    http_response_code($httpCode);
    header('Content-Type: ' . $contentType);
    echo json_encode($data);
    exit;
}

logMessage("--- Petición Recibida ---");

if ($_SERVER['REQUEST_METHOD'] !== $expectedMethod) {
    sendResponse(['status' => 'error', 'message' => 'Método no permitido. Solo se acepta ' . $expectedMethod . '.'], 405);
}

$receivedApiKey = $_SERVER['HTTP_' . strtoupper(str_replace('-', '_', $apiKeyHeader))] ?? '';

if ($receivedApiKey !== $expectedApiKey) {
    logMessage("Autenticación Fallida. Clave recibida: '$receivedApiKey'");
    sendResponse(['status' => 'error', 'message' => 'Acceso denegado. Clave de autenticación inválida.'], 401);
}
logMessage("Autenticación OK.");

if (isset($GLOBALS['HTTP_RAW_POST_DATA']) && !empty($GLOBALS['HTTP_RAW_POST_DATA'])) {
    $json_data = $GLOBALS['HTTP_RAW_POST_DATA'];
} else {
    $json_data = file_get_contents('php://input');
}

$data = json_decode($json_data, true);

if ($data === null) {
    sendResponse(['status' => 'error', 'message' => 'Cuerpo de la solicitud inválido. Se espera un JSON.'], 400);
}

foreach ($requiredFields as $field) {
    if (!isset($data[$field]) || empty(trim($data[$field]))) {
        sendResponse(['status' => 'error', 'message' => "Falta el campo requerido: '$field'."], 400);
    }
}

$deviceId = $data['device_id'];
$ipAddress = $data['ip_address'];
$currentTime = date('Y-m-d H:i:s'); 

$dsn = "mysql:host=$dbHost;dbname=$dbName;charset=utf8mb4";

try {
    $pdo = new PDO($dsn, $dbUser, $dbPass, [
        PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION, 
        PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
        PDO::ATTR_EMULATE_PREPARES => false,
    ]);
    logMessage("Conexión a la BD exitosa.");

} catch (PDOException $e) {
    logMessage("Error de conexión a la BD: " . $e->getMessage());
    sendResponse(['status' => 'error', 'message' => 'Error interno del servidor al conectar a la base de datos.'], 500);
}

$sql = "INSERT INTO $tableName (device_id, ip_address, fecha_registro, ultima_actualizacion) 
        VALUES (:device_id, :ip_address, :reg_time, :upd_time_insert)
        ON DUPLICATE KEY UPDATE 
            ip_address = :ip_address_update,
            ultima_actualizacion = :upd_time";

try {
    $stmt = $pdo->prepare($sql);
    
    $stmt->bindParam(':device_id', $deviceId);
    $stmt->bindParam(':ip_address', $ipAddress);
    
    $stmt->bindParam(':reg_time', $currentTime); 
    $stmt->bindParam(':upd_time_insert', $currentTime);

    $stmt->bindParam(':ip_address_update', $ipAddress);
    $stmt->bindParam(':upd_time', $currentTime);

    $stmt->execute();
    
    $action = ($stmt->rowCount() === 1) ? "Insertado" : "Actualizado";

    logMessage("Registro $action: DeviceID='{$deviceId}', IP='{$ipAddress}'");
    
    sendResponse([
        'status' => 'success',
        'message' => "IP $action exitosamente en la base de datos.",
        'action' => $action,
        'device_id' => $deviceId
    ], 200);

} catch (PDOException $e) {
    $errorMsg = "Error en la operación de BD: " . $e->getMessage();
    logMessage($errorMsg);
    sendResponse(['status' => 'error', 'message' => 'Error al ejecutar la consulta en la base de datos.'], 500);
}
