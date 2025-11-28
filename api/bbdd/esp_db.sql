
CREATE TABLE IF NOT EXISTS dispositivos_ip (
    -- Clave primaria auto-incremental para indexación rápida
    id INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,

    -- Clave principal de la lógica: Identificador único del dispositivo (ESP_TEST_012)
    device_id VARCHAR(50) NOT NULL,

    -- Datos a actualizar: La dirección IP actual
    ip_address VARCHAR(45) NOT NULL,
    
    -- Metadatos: Fecha de registro inicial y fecha de la última actualización
    fecha_registro DATETIME NOT NULL,
    ultima_actualizacion DATETIME NOT NULL,

    -- Definir device_id como CLAVE ÚNICA para permitir la cláusula ON DUPLICATE KEY UPDATE
    PRIMARY KEY (id),
    UNIQUE KEY (device_id) 

) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;