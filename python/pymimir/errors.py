"""Public Mimir exceptions."""

from __future__ import annotations


class MimirError(RuntimeError):
    """A native Mimir operation failed."""


class PddlError(MimirError):
    """A PDDL document could not be loaded."""

    document_type: str
    error_code: str
    source_path: str | None
    offset: int | None
    length: int | None
    line: int | None
    column: int | None

    def __init__(
        self,
        message: str,
        *,
        document_type: str,
        error_code: str,
        source_path: str | None,
        offset: int | None,
        length: int | None,
        line: int | None,
        column: int | None,
    ) -> None:
        super().__init__(message)
        self.document_type = document_type
        self.error_code = error_code
        self.source_path = source_path
        self.offset = offset
        self.length = length
        self.line = line
        self.column = column
