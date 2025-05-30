export interface Seeker {
    id: string;
    name: string;
    os: string;
    ip: string;
    last_active: string;
    disconnected: boolean;
    keylogs: string;
    metadata: Record<string, unknown>;
}

export interface Message {
    type: string;
    data: unknown;
}