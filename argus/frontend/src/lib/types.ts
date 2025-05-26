export interface Rat {
    id: string;
    name: string;
    os: string;
    ip: string;
    metadata: Record<string, unknown>;
}

export interface Message {
    type: string;
    data: unknown;
}