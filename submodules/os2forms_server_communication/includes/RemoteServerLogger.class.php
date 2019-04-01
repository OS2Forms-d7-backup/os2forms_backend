<?php

namespace Os2formsServerCommunication;

use Monolog\Formatter\LineFormatter;
use Monolog\Logger;

/**
 * RemoteServerLogger class definition.
 */
class RemoteServerLogger {
  /**
   * Wrapper for logging info message.
   *
   * @param string $message
   *   The message.
   */
  public static function info($message) {
    $replacements = func_get_args();
    array_shift($replacements);

    if (!empty($replacements)) {
      RemoteServerLogger::writeLogMessage(Logger::INFO, vsprintf($message, $replacements));
    }
    else {
      RemoteServerLogger::writeLogMessage(Logger::INFO, $message);
    }
  }

  /**
   * Wrapper for logging warning message.
   *
   * @param string $message
   *   The message.
   */
  public static function warning($message) {
    $replacements = func_get_args();
    array_shift($replacements);

    if (!empty($replacements)) {
      RemoteServerLogger::writeLogMessage(Logger::WARNING, vsprintf($message, $replacements));
    }
    else {
      RemoteServerLogger::writeLogMessage(Logger::WARNING, $message);
    }
  }

  /**
   * Wrapper for error warning message.
   *
   * @param string $message
   *   The message.
   */
  public static function error($message) {
    $replacements = func_get_args();
    array_shift($replacements);

    if (!empty($replacements)) {
      RemoteServerLogger::writeLogMessage(Logger::ERROR, vsprintf($message, $replacements));
    }
    else {
      RemoteServerLogger::writeLogMessage(Logger::ERROR, $message);
    }
  }

  /**
   * Calls actual logging command.
   *
   * @param int $level
   *   Level of logging.
   * @param string $message
   *   The messages.
   */
  private static function writeLogMessage($level, $message) {
    $logger = monolog('os2forms_server_communication_channel');
    $handler = $logger->popHandler();
    if ($handler) {
      // The default output format is
      // "[%datetime%] %channel%.%level_name%: %message% %context% %extra%\n".
      $output = "[%datetime%] %level_name%: %message%\n";
      $formatter = new LineFormatter($output);
      $handler->setFormatter($formatter);
      $logger->pushHandler($handler);
    }
    $logger->log($level, $message, array());
  }
}
