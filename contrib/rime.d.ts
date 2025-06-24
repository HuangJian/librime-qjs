// Type definitions for Rime QJS API
// Project: Rime QJS Plugin
// -----------------------------------
// @author https://github.com/HuangJian
// @version LIB_RIME_QJS_VERSION

/**
 * Represents a keyboard input event
 * @namespace KeyEvent
 */
interface KeyEvent {
  /**
   * Whether Shift modifier is active
   * @readonly
   */
  readonly shift: boolean

  /**
   * Whether Control modifier is active
   * @readonly
   */
  readonly ctrl: boolean

  /**
   * Whether Alt/Option modifier is active
   * @readonly
   */
  readonly alt: boolean

  /**
   * True if this is a key release event
   * @readonly
   */
  readonly release: boolean

  /**
   * String representation of the key event (e.g. 'Return')
   * @readonly
   */
  readonly repr: string
}

/**
 * Represents composition state during input
 * @namespace Preedit
 */
interface Preedit {
  /**
   * Current composition text
   * @returns {string} Composed text
   */
  text: string

  /**
   * Current caret position within the preedit
   * @returns {number} Caret index position
   */
  caretPos: number

  /**
   * Selection start index in the preedit
   * @readonly
   */
  selectStart: number

  /**
   * Selection end index in the preedit
   * @readonly
   */
  selectEnd: number
}

/**
 * Represents a segment of the composition
 * @namespace Segment
 */
interface Segment {
  /**
   * Index of currently selected candidate
   * @returns {number} Current selection index
   */
  selectedIndex: number

  /**
   * Prompt text displayed before candidates
   * @returns {string} Prompt message
   */
  prompt: string

  /**
   * Start position in the input sequence
   * @readonly
   */
  readonly start: number

  /**
   * End position in the input sequence
   * @readonly
   */
  readonly end: number

  /** Currently selected candidate */
  readonly selectedCandidate: Candidate
  /** Number of candidates in this segment */
  readonly candidateSize: number

  /**
   * Get candidate at specified index
   * @param index - Zero-based candidate index
   * @returns Candidate object or null if index is out of bounds
   */
  getCandidateAt(index: number): Candidate | null

  /**
   * Check if segment contains specific tag
   * @param tag - Tag to check for
   * @returns True if segment has the specified tag
   */
  hasTag(tag: string): boolean
}

/**
 * Manages input context and composition state
 * @namespace Context
 */
interface Context {
  /**
   * Current raw input string
   * @returns {string} Unprocessed input
   */
  input: string

  /**
   * Current caret position in the input
   * @returns {number} Caret index position
   */
  caretPos: number

  /** Current composition state */
  readonly preedit: Preedit
  /** Last segment in current composition (null if hasMenu() is false) */
  readonly lastSegment: Segment | null
  /** Notifier for commit events */
  readonly commitNotifier: Notifier
  /** Notifier for selection events */
  readonly selectNotifier: Notifier
  /** Notifier for update events */
  readonly updateNotifier: Notifier
  /** Notifier for delete events */
  readonly deleteNotifier: Notifier
  /** The commit history */
  readonly commitHistory: CommitHistory

  /** Commit current composition */
  commit(): void

  /**
   * Get text that would be committed
   * @returns Currently composed text
   */
  getCommitText(): string

  /** Clear input and composition state */
  clear(): void

  /**
   * Check if candidate menu is available
   * @returns True if menu has candidates
   */
  hasMenu(): boolean
}

/**
 * Represents an input schema configuration
 * @namespace Schema
 */
interface Schema {
  /**
   * Unique schema identifier string
   * @readonly
   */
  readonly id: string

  /**
   * Display name of the schema
   * @readonly
   */
  readonly name: string

  /**
   * Configuration object containing schema settings
   * @readonly
   */
  readonly config: Config

  /**
   * Number of candidates shown per page
   * @readonly
   */
  readonly pageSize: number

  /**
   * Key sequence used for candidate selection
   * @readonly
   */
  readonly selectKeys: string
}

/**
 * Core input method engine
 * @namespace Engine
 */
interface Engine {
  /**
   * Currently active schema
   * @readonly
   */
  readonly schema: Schema

  /**
   * Input context
   * @readonly
   */
  readonly context: Context

  /** Reference to parent engine (null for root engine) */
  readonly activeEngine: Engine | null

  /**
   * Process a key event
   * @param keyEvent - String representation of key event (e.g. 'Down')
   * @returns True if the key was processed, false if ignored
   */
  processKey(keyEvent: string): boolean

  /**
   * Commit text directly to output
   * @param text - Text to commit
   */
  commitText(text: string): void

  /**
   * Switch to a different schema
   * @param schema - Target schema to activate
   * @returns True if schema switch was successful
   */
  applySchema(schema: Schema): boolean
}

/**
 * Represents a configuration value with type-specific accessors
 * @namespace ConfigValue
 */
interface ConfigValue {
  /**
   * Get the value type (always returns 'scalar')
   * @returns {'scalar'} Type identifier
   */
  getType(): 'scalar'

  /**
   * Retrieve boolean value from configuration
   * @returns {boolean | null} Configured value or null if not found
   */
  getBool(): boolean | null

  /**
   * Retrieve integer value from configuration
   * @returns {number | null} Configured value or null if not found
   */
  getInt(): number | null

  /**
   * Retrieve floating-point value from configuration
   * @returns {number | null} Configured value or null if not found
   */
  getDouble(): number | null

  /**
   * Retrieve string value from configuration
   * @returns {string | null} Configured value or null if not found
   */
  getString(): string | null
}

/**
 * Represents a configuration item that can hold different value types
 * @namespace ConfigItem
 */
interface ConfigItem {
  /**
   * Get the value type (always returns 'item')
   * @returns {'item'} Type identifier
   */
  getType(): 'item'
}

/**
 * Represents a configuration list structure
 * @namespace ConfigList
 */
interface ConfigList {
  /**
   * Get the container type (always returns 'list')
   * @returns {'list'} Type identifier
   */
  getType(): 'list'

  /**
   * Get number of items in the list
   * @returns {number} List size
   */
  getSize(): number

  /**
   * Get configuration item at specified index
   * @param index - Zero-based index position
   * @returns {ConfigItem | null} Item or null if index out of bounds
   */
  getItemAt(index: number): ConfigItem | null

  /**
   * Get configuration value at specified index
   * @param index - Zero-based index position
   * @returns {ConfigValue | null} Value or null if index out of bounds
   */
  getValueAt(index: number): ConfigValue | null

  /**
   * Append new item to the end of the list
   * @param item - Configuration item to add
   */
  pushBack(item: ConfigItem): void

  /** Clear all items from the list */
  clear(): void
}

/**
 * Represents a configuration map/dictionary structure
 * @namespace ConfigMap
 */
interface ConfigMap {
  /**
   * Get the container type (always returns 'map')
   * @returns {'map'} Type identifier
   */
  getType(): 'map'

  /**
   * Check if map contains a specific key
   * @param key - Key to check for existence
   * @returns {boolean} True if key exists in the map
   */
  hasKey(key: string): boolean

  /**
   * Get configuration item by key
   * @param key - Map key to lookup
   * @returns {ConfigItem | null} Associated item or null if not found
   */
  getItem(key: string): ConfigItem | null

  /**
   * Get configuration value by key
   * @param key - Map key to lookup
   * @returns {ConfigValue | null} Associated value or null if not found
   */
  getValue(key: string): ConfigValue | null

  /**
   * Set/update a key-value pair in the map
   * @param key - Map key to set
   * @param item - Configuration item to associate with the key
   */
  setItem(key: string, item: ConfigItem): void
}

/**
 * Root configuration container with file operations
 * @namespace Config
 */
interface Config {
  /**
   * Load configuration from file
   * @param filePath - Path to configuration file
   * @returns {boolean} True if load succeeded
   */
  loadFromFile(filePath: string): boolean

  /**
   * Save configuration to file
   * @param filePath - Destination file path
   * @returns {boolean} True if save succeeded
   */
  saveToFile(filePath: string): boolean

  /**
   * Get boolean value by path
   * @param path - Configuration path (e.g. 'menu/page_size')
   * @returns {boolean | null} Configured value or null if not found
   */
  getBool(path: string): boolean | null

  /**
   * Get integer value by path
   * @param path - Configuration path
   * @returns {number | null} Configured value or null if not found
   */
  getInt(path: string): number | null

  /**
   * Get double value by path
   * @param path - Configuration path
   * @returns {number | null} Configured value or null if not found
   */
  getDouble(path: string): number | null

  /**
   * Get string value by path
   * @param path - Configuration path
   * @returns {string | null} Configured value or null if not found
   */
  getString(path: string): string | null

  /**
   * Get configuration list by path
   * @param path - Configuration path
   * @returns {ConfigList | null} List container or null if not found
   */
  getList(path: string): ConfigList | null

  /**
   * Set boolean value by path
   * @param path - Configuration path
   * @param value - New value to set
   * @returns {boolean} True if update succeeded
   */
  setBool(path: string, value: boolean): boolean

  /**
   * Set integer value by path
   * @param path - Configuration path
   * @param value - New value to set
   * @returns {boolean} True if update succeeded
   */
  setInt(path: string, value: number): boolean

  /**
   * Set double value by path
   * @param path - Configuration path
   * @param value - New value to set
   * @returns {boolean} True if update succeeded
   */
  setDouble(path: string, value: number): boolean

  /**
   * Set string value by path
   * @param path - Configuration path
   * @param value - New value to set
   * @returns {boolean} True if update succeeded
   */
  setString(path: string, value: string): boolean
}

/**
 * Represents a candidate entry in the selection menu
 * @namespace Candidate
 */
interface Candidate {
  /**
   * Create a new Candidate instance
   * @param type - Candidate type identifier (e.g. 'pinyin', 'stroke')
   * @param start - Start position in input sequence (zero-based index)
   * @param end - End position in input sequence (zero-based index)
   * @param text - Display text for the candidate
   * @param comment - Annotation/comment text (often pronunciation)
   * @param quality - Optional quality score (higher = better match)
   */
  new (type: string, start: number, end: number, text: string, comment: string, quality?: number): Candidate

  /** The displayed candidate text */
  text: string
  /** Phonetic annotation or explanatory comment */
  comment: string
  /** Candidate type identifier */
  type: string
  /** Start index in input sequence */
  start: number
  /** End index in input sequence */
  end: number
  /** Match quality score (0-100) */
  quality: number
  /** The user input characters corresponding to the current candidate */
  preedit?: string
}

/**
 * Represents a candidate iterator
 * @namespace CandidateIterator
 */
interface CandidateIterator {
  /**
   * Get the next candidate or null if at the end
   */
  next(): Candidate | null
}

/**
 * Options for parsing text files
 * @namespace ParseTextFileOptions
 */
interface ParseTextFileOptions {
  /**
   * Whether the key-value pairs in the file are in reversed order (value\tkey)
   * @default false
   */
  isReversed?: boolean

  /**
   * Characters to remove from each line before processing
   * @default ""
   */
  charsToRemove?: string

  /**
   * Maximum number of lines to read from the file
   * @default undefined - reads all lines
   */
  lines?: number

  /**
   * Character(s) used to separate fields in each line
   * @default "\t" - tab character
   */
  delimiter?: string

  /**
   * Character(s) used to mark comment lines
   * @default "#"
   */
  comment?: string

  /**
   * strategy to handle duplicated keys
   * @default "Overwrite"
   */
  onDuplicatedKey?: 'Overwrite' | 'Skip' | 'Concat'

  /**
   * The separator to use when concatenating values for duplicated keys
   * @default "$|$"
   */
  concatSeparator?: string
}

/**
 * Represents a dictionary trie data structure, to store the dictionary data (in key-value pairs)
 */
interface Trie {
  /**
   * Creates a new instance of Trie
   */
  new (): Trie

  /**
   * Loads a trie from a text file.
   *    - Each line in the file should contain a key-value pair separated by a tab character.
   *    - The key and value should be separated by a tab character: `key\tvalue`.
   *    - Modify the parse text file options for customized format.
   * @param path - The path to the text file containing trie data
   * @param options - Options for parsing the text file
   * @throws {Error} If the file cannot be read or the format is invalid
   */
  loadTextFile(path: string, options?: ParseTextFileOptions): void

  /**
   * Loads a trie from a binary file
   * @param path - The path to the binary file containing trie data
   * @throws {Error} If the file cannot be read or the format is invalid
   */
  loadBinaryFile(path: string): void

  /**
   * Saves the current trie to a binary file
   * @param path - The path where the binary file will be saved
   * @throws {Error} If the file cannot be written
   */
  saveToBinaryFile(path: string): void

  /**
   * Searches for an exact match of the key in the trie
   * @param key - The string to search for
   * @returns the value if the key exists in the trie, null otherwise
   */
  find(key: string): string

  /**
   * Searches for all key-value pairs in the trie that the key starts with the given prefix
   * @param prefix - The prefix to search for
   * @returns An array of objects that the key starts with the given prefix
   */
  prefixSearch(prefix: string): Array<{ text: string; info: string }>
}

/**
 * Represents system information about the host operating system where Rime is running
 * @namespace SystemInfo
 * @description Provides read-only access to basic system properties including OS name, version and architecture
 */
interface SystemInfo {
  /**
   * The operating system name
   * @readonly
   */
  readonly name: 'macOS' | 'Windows' | 'Linux' | 'Unknown Linux' | 'Unknown OS'

  /**
   * The operating system version string
   * @example "10.15.7", "11.0", "22.04"
   * @readonly
   */
  readonly version: string

  /**
   * The system CPU architecture
   * @example "x86_64", "arm64", "amd64", "Unknown"
   * @readonly
   */
  readonly architecture: string
}

/**
 * Represents the JavaScript runtime environment for Rime
 * @namespace Environment
 */
interface Environment {
  /**
   * A unique ID that identifies this environment instance, combining the plugin identifier and active Rime session
   * @readonly
   */

  readonly id: string
  /**
   * Reference to the Rime engine instance
   * @readonly
   */
  readonly engine: Engine

  /**
   * Current namespace identifier
   * @readonly
   */
  readonly namespace: string

  /**
   * Path to the user data directory
   * @remarks This path is frontend-specific as documented in https://github.com/rime/home/wiki/UserData#%E4%BD%8D%E7%BD%AE
   * @example Squirrel on macOS: `~/Library/Rime`
   * @example Weasel on Windows: `%APPDATA%\Rime`
   * @example Fcitx5 on Linux: `~/.local/share/fcitx5/rime/`
   * @readonly
   */
  readonly userDataDir: string

  /**
   * Path to the shared data directory
   * @remarks This path is frontend-specific as documented in https://github.com/rime/home/wiki/SharedData#%E4%BD%8D%E7%BD%AE
   * @example Squirrel on macOS: `"/Library/Input Methods/Squirrel.app/Contents/SharedSupport"`
   * @example Weasel on Windows: `<安裝目錄>\data`
   * @example Fcitx on Linux: `/usr/share/rime-data`
   * @readonly
   */
  readonly sharedDataDir: string

  /**
   * The system information of the host operating system
   * @readonly
   */
  readonly os: SystemInfo

  /**
   * Load content from a file
   * @param absolutePath - Full path to the file to load
   * @returns {string} File contents as string
   * @throws {Error} If file cannot be read or path is invalid
   */
  loadFile(absolutePath: string): string

  /**
   * Check if a file exists
   * @param absolutePath - Full path to check
   * @returns {boolean} True if file exists
   */
  fileExists(absolutePath: string): boolean

  /**
   * Get Rime version and memory usage information
   * @returns {string} Formatted string with version and memory stats
   */
  getRimeInfo(): string

  /**
   * Execute a shell command and capture its output
   * @param command - Shell command to execute
   * @param timeout - Max duration in milliseconds to execute the command, default to 1000ms
   * @returns {string} Command output
   * @throws {Error} If command execution fails or timed-out
   */
  popen(command: string, timeout?: number): string
}

/**
 * Interface for JavaScript module lifecycle in Rime
 * @namespace Module
 */
declare class Module {
  /**
   * Constructor function called when module is loaded
   * @param env - The runtime environment
   */
  constructor(env: Environment)

  /**
   * Cleanup function called when module is unloaded
   * @returns {void}
   */
  finalizer?(): void
}

/**
 * Return values for key event processing
 * @remarks
 * - 'kRejected': do the OS default processing
 * - 'kAccepted': consume it
 * - 'kNoop': leave it to other processors
 */
type ProcessResult = 'kRejected' | 'kAccepted' | 'kNoop'

/**
 * Represents a Rime input processor
 * @namespace Processor
 */
declare class Processor extends Module {
  /**
   * Process a keyboard input event
   * @param keyEvent - The key event to process
   * @param env - The runtime environment
   * @returns {ProcessResult} The processing result
   */
  process(keyEvent: KeyEvent, env: Environment): ProcessResult
}

/**
 * Represents a Rime translator module for converting input to candidates
 * @namespace Translator
 */
declare class Translator extends Module {
  /**
   * Translate to candidates by the input
   * @param input - The input string to translate
   * @param segment - The current segment being translated
   * @param env - The runtime environment
   * @returns {Array<Candidate>} Array of translated candidates
   */
  translate(input: string, segment: Segment, env: Environment): Array<Candidate>
}

/**
 * Represents a Rime filter module for processing candidates
 * @namespace Filter
 */
declare class Filter extends Module {
  /**
   * Check if the filter is applicable in the current context
   * @param env - The runtime environment
   * @returns {boolean} True if the filter is applicable
   */
  isApplicable?(env: Environment): boolean

  /**
   * Apply filtering to a list of candidates
   * @param candidates - Array of candidates to filter
   * @param env - The runtime environment
   * @returns {Array<Candidate>} Filtered array of candidates
   */
  filter(candidates: Array<Candidate>, env: Environment): Array<Candidate>
}

/**
 * Represents a Rime filter module for processing candidates fastly with a candidate iterator
 * @namespace FastFilter
 */
declare class FastFilter extends Module {
  /**
   * Check if the filter is applicable in the current context
   * @param env - The runtime environment
   * @returns {boolean} True if the filter is applicable
   */
  isApplicable?(env: Environment): boolean
  /**
   * Apply filtering to the candidates accessible through the iterator
   * @param iterator - The candidate iterator
   * @param env - The runtime environment
   * @returns {Generator<Candidate, CandidateIterator | void>} Generator yielding filtered candidates
   */
  *filter(iterator: CandidateIterator, env: Environment): Generator<Candidate, CandidateIterator | void>
}

/**
 * Represents a notification signal for context changes
 * @namespace Notifier
 */
interface Notifier {
  /**
   * Connect a listener function to receive context change notifications
   * @param listener - Callback function that receives the updated Context
   * @returns {NotifierConnection} Connection object that can be used to disconnect the listener
   */
  connect(listener: (context: Context) => void): NotifierConnection
}

/**
 * Represents a connection to a Notifier
 * @namespace NotifierConnection
 */
interface NotifierConnection {
  /**
   * Disconnect this listener from receiving notifications
   */
  disconnect(): void

  /**
   * Check if the connection is still active
   * @readonly
   */
  readonly isConnected: boolean
}

/**
 * Represents a commit record entry
 * @namespace CommitRecord
 */
interface CommitRecord {
  /** The type of the commit record */
  type: string
  /** The text content of the commit record */
  text: string
}

/**
 * Represents a history of commit records
 * @namespace CommitHistory
 */
interface CommitHistory {
  /**
   * Push a new commit record to history
   * @param type - Type identifier for the commit
   * @param text - Text content to commit
   */
  push(type: string, text: string): void

  /**
   * Get the last commit record
   * @returns {CommitRecord | undefined} The most recent commit record or undefined if history is empty
   */
  readonly last: CommitRecord | undefined

  /**
   * Get string representation of commit history
   * @returns {string} Formatted history string
   */
  readonly repr: string
}

/**
 * Represents a LevelDB key-value store for dictionary data
 * @namespace LevelDb
 */
interface LevelDb {
  /**
   * Creates a new instance of LevelDb
   */
  new (): LevelDb

  /**
   * Loads dictionary data from a text file.
   * Each line in the file should contain a key-value pair separated by a delimiter.
   * @param path - The path to the text file containing dictionary data
   * @param options - Options for parsing the text file
   * @throws {Error} If the file cannot be read or the format is invalid
   */
  loadTextFile(path: string, options?: ParseTextFileOptions): void

  /**
   * Loads dictionary data from a binary file
   * @param path - The path to the binary file containing dictionary data
   * @throws {Error} If the file cannot be read or the format is invalid
   */
  loadBinaryFile(path: string): void

  /**
   * Saves the current dictionary data to a binary file
   * @param path - The path where the binary file will be saved
   * @throws {Error} If the file cannot be written
   */
  saveToBinaryFile(path: string): void

  /**
   * Searches for an exact match of the key in the dictionary
   * @param key - The string to search for
   * @returns the value if the key exists in the dictionary, null otherwise
   */
  find(key: string): string | null

  /**
   * Searches for all key-value pairs in the dictionary where the key starts with the given prefix
   * @param prefix - The prefix to search for
   * @returns An array of objects containing matching key-value pairs
   */
  prefixSearch(prefix: string): Array<{ text: string; info: string }>

  /**
   * Closes the LevelDB database and releases resources
   */
  close(): void
}
